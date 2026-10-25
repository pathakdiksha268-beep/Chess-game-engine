/* ============================================================
   Chess Engine website
   The chess rules and the AI live in the C++ engine (chess.wasm).
   This file only draws the board and passes clicks to the engine.
   ============================================================ */
(() => {
  'use strict';

  /* ---------- constants ---------- */
  const FILES = 'abcdefgh';
  const START = 'rnbqkbnrpppppppp................................PPPPPPPPRNBQKBNR';
  const GLYPH = { k: '\u265A', q: '\u265B', r: '\u265C', b: '\u265D', n: '\u265E', p: '\u265F' };
  const PIECE_NAME = { k: 'king', q: 'queen', r: 'rook', b: 'bishop', n: 'knight', p: 'pawn' };
  const VALUE = { p: 1, n: 3, b: 3, r: 5, q: 9 };
  const START_COUNT = { p: 8, n: 2, b: 2, r: 2, q: 1 };
  const MATE = 100000;              // the engine scores a forced mate as about +/- this
  const ARROW_OPACITY = [0.85, 0.5, 0.3];
  const ARROW_WIDTH = [0.17, 0.13, 0.10];

  const THEMES = [
    { id: 'classic',  name: 'Classic' },
    { id: 'emerald',  name: 'Emerald' },
    { id: 'ocean',    name: 'Ocean' },
    { id: 'amethyst', name: 'Amethyst' },
    { id: 'midnight', name: 'Midnight' },
  ];
  // search depth = how many moves ahead the AI looks
  const LEVELS = {
    easy:   { depth: 1, name: 'Beginner',     hint: 'Looks 1 move ahead. A good first opponent.' },
    medium: { depth: 2, name: 'Intermediate', hint: 'Looks 2 moves ahead. Plays sensible chess.' },
    hard:   { depth: 3, name: 'Advanced',     hint: 'Looks 3 moves ahead. Thinks for a moment.' },
  };

  /* ---------- tiny helpers ---------- */
  const $ = (sel) => document.querySelector(sel);
  const $$ = (sel) => Array.from(document.querySelectorAll(sel));
  const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
  const sqName = (r, c) => FILES[c] + (8 - r);
  const isWhitePiece = (ch) => ch === ch.toUpperCase();

  const el = {
    note: $('#engine-note'),
    board: $('#board'),
    preview: $('#preview-board'),
    themeList: $('#theme-list'),
    setupSub: $('#setup-sub'),
    previewCaption: $('#preview-caption'),
    levelHint: $('#level-hint'),
    aiOptions: $('#ai-options'),
    localOptions: $('#local-options'),
    optCoords: $('#opt-coords'),
    optHints: $('#opt-hints'),
    optRotate: $('#opt-rotate'),
    segColor: $('#seg-color'),
    segLevel: $('#seg-level'),
    moveList: $('#move-list'),
    statusMain: $('#status-main'),
    statusSub: $('#status-sub'),
    turnDot: $('#turn-dot'),
    btnUndo: $('#btn-undo'),
    btnHint: $('#btn-hint'),
    levelOptions: $('#level-options'),
    levelTitle: $('#level-title'),
    evalBar: $('#evalbar'),
    evalWhite: $('#eval-white'),
    arrows: $('#arrows'),
    analysisDepth: $('#analysis-depth'),
    analysisEmpty: $('#analysis-empty'),
    analysisBody: $('#analysis-body'),
    analysisScore: $('#analysis-score'),
    analysisText: $('#analysis-text'),
    analysisLines: $('#analysis-lines'),
    modal: $('#modal'),
    modalCard: $('#modal-card'),
  };

  /* ---------- saved settings ---------- */
  const DEFAULTS = { theme: 'classic', coords: true, hints: true, rotate: false, color: 'white', level: 'medium' };

  function loadSettings() {
    let saved = {};
    try { saved = JSON.parse(localStorage.getItem('chess.settings') || '{}') || {}; } catch (e) { saved = {}; }
    const s = Object.assign({}, DEFAULTS, saved);
    if (!THEMES.some((t) => t.id === s.theme)) s.theme = DEFAULTS.theme;
    if (!['white', 'black', 'random'].includes(s.color)) s.color = DEFAULTS.color;
    if (!LEVELS[s.level]) s.level = DEFAULTS.level;
    return s;
  }
  function saveSettings() {
    try { localStorage.setItem('chess.settings', JSON.stringify(settings)); } catch (e) { /* private mode */ }
  }
  const settings = loadSettings();

  /* ---------- game state ---------- */
  const state = {
    mode: 'ai',          // 'ai' or 'local'
    humanColor: 0,       // 0 = White, 1 = Black (only used against the AI)
    depth: 2,
    levelName: '',
    inGame: false,
    busy: false,         // true while the AI is thinking (or looking for a hint)
    working: 'ai',       // what the engine is busy with: 'ai' or 'hint'
    hint: null,          // the best move found by the Hint button, or null
    analysis: null,      // { moves: [best, second, third], depth } from the Hint button, or null
    flipped: false,
    animate: false,
    board: START,        // 64 characters from the engine
    turn: 0,
    status: 0,           // 0 normal, 1 check, 2 checkmate, 3 stalemate
    legal: [],
    selected: null,
    lastMove: null,
    history: [],         // { move, san, mover }
  };

  let engine = null;
  let gameSquares = [];
  let previewSquares = [];
  let aiToken = 0;       // lets us cancel a pending AI move

  /* ============================================================
     Talking to the C++ engine
     ============================================================ */
  async function loadEngine() {
    if (typeof createChessModule !== 'function') throw new Error('chess.js was not loaded');
    const M = await createChessModule();
    const call = (name, ret, args) => M.cwrap(name, ret, args);
    return {
      newGame: call('engine_new_game', null, []),
      board: call('engine_board', 'string', []),
      turn: call('engine_turn', 'number', []),
      status: call('engine_status', 'number', []),
      legal: call('engine_legal_moves', 'string', []),
      play: call('engine_play', 'number', ['number', 'number', 'number', 'number', 'number']),
      ai: call('engine_ai_move', 'string', ['number']),
      analyze: call('engine_analyze', 'string', ['number', 'number']),
      undo: call('engine_undo', 'number', []),
    };
  }

  // "6,4,4,4,n,-" -> { fr, fc, tr, tc, type, promo }
  function parseMove(text) {
    const p = text.split(',');
    return { fr: +p[0], fc: +p[1], tr: +p[2], tc: +p[3], type: p[4], promo: p[5] };
  }
  function parseMoves(text) {
    return text ? text.split(';').map(parseMove) : [];
  }

  // Copy the engine's current position into our state
  function syncFromEngine() {
    state.board = engine.board();
    state.turn = engine.turn();
    state.status = engine.status();
    state.legal = state.status >= 2 ? [] : parseMoves(engine.legal());
  }

  /* ============================================================
     Move notation (e4, Nf3, O-O, exd6, e8=Q+ ...)
     ============================================================ */
  function toSan(m, before, legalBefore, statusAfter) {
    const piece = before[m.fr * 8 + m.fc];
    const kind = piece.toLowerCase();
    let s;
    if (m.type === 'c') {
      s = m.tc > m.fc ? 'O-O' : 'O-O-O';
    } else {
      const capture = before[m.tr * 8 + m.tc] !== '.' || m.type === 'e';
      if (kind === 'p') {
        s = (capture ? FILES[m.fc] + 'x' : '') + sqName(m.tr, m.tc);
        if (m.type === 'p') s += '=' + m.promo.toUpperCase();
      } else {
        s = kind.toUpperCase();
        // if two identical pieces can reach the square, say which one moved
        const rivals = legalBefore.filter((o) =>
          (o.fr !== m.fr || o.fc !== m.fc) && o.tr === m.tr && o.tc === m.tc &&
          before[o.fr * 8 + o.fc] === piece);
        if (rivals.length) {
          const sameFile = rivals.some((o) => o.fc === m.fc);
          const sameRank = rivals.some((o) => o.fr === m.fr);
          if (!sameFile) s += FILES[m.fc];
          else if (!sameRank) s += String(8 - m.fr);
          else s += FILES[m.fc] + (8 - m.fr);
        }
        if (capture) s += 'x';
        s += sqName(m.tr, m.tc);
      }
    }
    if (statusAfter === 2) s += '#';
    else if (statusAfter === 1) s += '+';
    return s;
  }

  /* ============================================================
     Material: captured pieces and who is ahead
     ============================================================ */
  function materialInfo(boardStr) {
    const cnt = { w: { p: 0, n: 0, b: 0, r: 0, q: 0 }, b: { p: 0, n: 0, b: 0, r: 0, q: 0 } };
    for (const ch of boardStr) {
      const kind = ch.toLowerCase();
      if (ch === '.' || kind === 'k') continue;
      cnt[isWhitePiece(ch) ? 'w' : 'b'][kind]++;
    }
    const missing = {};
    const total = {};
    for (const side of ['w', 'b']) {
      const m = {};
      let extra = 0;                                   // promoted pieces
      let sum = 0;
      for (const t of 'qrbn') {
        const d = START_COUNT[t] - cnt[side][t];
        if (d > 0) m[t] = d; else extra += -d;
      }
      m.p = Math.max(0, 8 - cnt[side].p - extra);      // a promoted pawn is not "captured"
      for (const t of 'pnbrq') sum += cnt[side][t] * VALUE[t];
      missing[side] = m;
      total[side] = sum;
    }
    return { missing, diff: total.w - total.b };
  }

  /* ============================================================
     Drawing a board (used for the game and for the preview)
     ============================================================ */
  function buildGrid(container) {
    container.textContent = '';
    const squares = [];
    for (let i = 0; i < 64; i++) {
      const d = document.createElement('div');
      d.className = 'sq';
      container.appendChild(d);
      squares.push(d);
    }
    return squares;
  }

  // v = { flipped, selected, last: {from,to}, check, targets: Map, animate }
  function paintBoard(squares, boardStr, v) {
    for (let i = 0; i < 8; i++) {
      for (let j = 0; j < 8; j++) {
        const r = v.flipped ? 7 - i : i;
        const c = v.flipped ? 7 - j : j;
        const key = r * 8 + c;
        const sq = squares[i * 8 + j];
        sq.dataset.r = r;
        sq.dataset.c = c;

        let cls = 'sq ' + ((r + c) % 2 === 0 ? 'light' : 'dark');
        if (v.selected === key) cls += ' sel';
        else if (v.last && (v.last.from === key || v.last.to === key)) cls += ' last';
        if (v.check === key) cls += ' check';
        if (v.hint && v.hint.from === key) cls += ' hint-from';
        if (v.hint && v.hint.to === key) cls += ' hint-to';
        const marker = v.targets && v.targets.get(key);
        if (marker) cls += ' ' + marker;
        sq.className = cls;
        sq.textContent = '';

        const ch = boardStr[key];
        let label = sqName(r, c);
        if (ch !== '.') {
          const white = isWhitePiece(ch);
          const p = document.createElement('span');
          p.className = 'piece ' + (white ? 'w' : 'b') +
            (v.animate && v.last && v.last.to === key ? ' land' : '');
          p.textContent = GLYPH[ch.toLowerCase()];
          sq.appendChild(p);
          label += ', ' + (white ? 'white ' : 'black ') + PIECE_NAME[ch.toLowerCase()];
        }
        sq.setAttribute('aria-label', label);

        if (j === 0) {
          const rank = document.createElement('span');
          rank.className = 'coord rank';
          rank.textContent = String(8 - r);
          sq.appendChild(rank);
        }
        if (i === 7) {
          const file = document.createElement('span');
          file.className = 'coord file';
          file.textContent = FILES[c];
          sq.appendChild(file);
        }
      }
    }
  }

  function paintPreview() {
    paintBoard(previewSquares, START, { flipped: false });
  }

  /* ============================================================
     Drawing the game screen
     ============================================================ */
  function render() {
    if (!state.inGame) return;

    // dots and rings for the selected piece's legal moves
    const targets = new Map();
    if (settings.hints && state.selected !== null && !state.busy) {
      for (const m of state.legal) {
        if (m.fr * 8 + m.fc !== state.selected) continue;
        const key = m.tr * 8 + m.tc;
        targets.set(key, (state.board[key] !== '.' || m.type === 'e') ? 'capture' : 'target');
      }
    }
    const lm = state.lastMove;
    const last = lm ? { from: lm.fr * 8 + lm.fc, to: lm.tr * 8 + lm.tc } : null;

    // red glow under a king in check
    let check = null;
    if (state.status === 1 || state.status === 2) {
      const idx = state.board.indexOf(state.turn === 0 ? 'K' : 'k');
      if (idx >= 0) check = idx;
    }

    const h = state.hint;
    const hint = h ? { from: h.fr * 8 + h.fc, to: h.tr * 8 + h.tc } : null;

    paintBoard(gameSquares, state.board, {
      flipped: state.flipped, selected: state.selected, last, check, targets, hint, animate: state.animate,
    });
    renderPlayers();
    renderStatus();
    renderMoves();
    el.btnUndo.disabled = undoSteps() === 0;
    el.btnHint.disabled = !canHint();
    renderAnalysis();
    renderArrows();
  }

  function renderPlayers() {
    const info = materialInfo(state.board);
    const topColor = state.flipped ? 0 : 1;
    fillBar('top', topColor, info);
    fillBar('bottom', 1 - topColor, info);
  }

  function fillBar(pos, color, info) {
    const bar = $('#bar-' + pos);
    const mine = color === 0 ? 'w' : 'b';
    const theirs = color === 0 ? 'b' : 'w';

    let name;
    if (state.mode === 'ai') name = color === state.humanColor ? 'You' : 'Computer \u00b7 ' + state.levelName;
    else name = color === 0 ? 'White' : 'Black';
    $('#name-' + pos).textContent = name;

    const avatar = $('#avatar-' + pos);
    avatar.className = 'avatar ' + mine;
    avatar.textContent = GLYPH.k;

    // pieces this side has captured = the opponent's missing pieces
    const capEl = $('#cap-' + pos);
    capEl.textContent = '';
    const missing = info.missing[theirs];
    for (const t of 'qrbnp') {
      for (let n = 0; n < (missing[t] || 0); n++) {
        const s = document.createElement('span');
        s.className = 'cap ' + theirs;
        s.textContent = GLYPH[t];
        capEl.appendChild(s);
      }
    }
    const lead = color === 0 ? info.diff : -info.diff;
    $('#adv-' + pos).textContent = lead > 0 ? '+' + lead : '';

    bar.classList.toggle('turn', state.status < 2 && state.turn === color);
  }

  function renderStatus() {
    let main;
    let sub = '';
    el.turnDot.className = 'dot ' + (state.turn === 0 ? 'w' : 'b');

    if (state.status === 2) {
      main = 'Checkmate';
      if (state.mode === 'ai') sub = (1 - state.turn === state.humanColor) ? 'You win!' : 'The computer wins.';
      else sub = (state.turn === 0 ? 'Black' : 'White') + ' wins.';
    } else if (state.status === 3) {
      main = 'Stalemate';
      sub = 'The game is a draw.';
    } else if (state.busy) {
      main = state.working === 'hint' ? 'Finding a hint\u2026' : 'Computer is thinking\u2026';
    } else {
      if (state.mode === 'ai') {
        main = state.turn === state.humanColor ? 'Your move' : "Computer's move";
        sub = 'You are playing ' + (state.humanColor === 0 ? 'White' : 'Black') + '.';
      } else {
        main = (state.turn === 0 ? 'White' : 'Black') + ' to move';
      }
      if (state.status === 1) sub = 'Check!';
      if (state.hint) {
        sub = (state.status === 1 ? 'Check! ' : '') + 'Hint: ' + state.hint.san +
              ' (' + sqName(state.hint.fr, state.hint.fc) + ' to ' + sqName(state.hint.tr, state.hint.tc) + ')';
      }
    }
    el.statusMain.textContent = main;
    el.statusSub.textContent = sub;
  }

  function renderMoves() {
    el.moveList.textContent = '';
    for (let i = 0; i < state.history.length; i += 2) {
      const li = document.createElement('li');
      const no = document.createElement('span');
      no.className = 'no';
      no.textContent = (i / 2 + 1) + '.';
      const w = document.createElement('span');
      w.textContent = state.history[i].san;
      const b = document.createElement('span');
      b.textContent = state.history[i + 1] ? state.history[i + 1].san : '';
      li.append(no, w, b);
      el.moveList.appendChild(li);
    }
    el.moveList.scrollTop = el.moveList.scrollHeight;
  }

  /* ============================================================
     Playing moves
     ============================================================ */
  function humanCanMove() {
    if (state.busy || state.status >= 2) return false;
    return state.mode === 'local' || state.turn === state.humanColor;
  }

  function onBoardClick(e) {
    const sq = e.target.closest('.sq');
    if (!sq || !state.inGame || !humanCanMove()) return;
    const r = +sq.dataset.r;
    const c = +sq.dataset.c;
    const key = r * 8 + c;

    // second click: is it a destination of the selected piece?
    if (state.selected !== null) {
      const options = state.legal.filter((m) =>
        m.fr * 8 + m.fc === state.selected && m.tr * 8 + m.tc === key);
      if (options.length) {
        if (options[0].type === 'p') askPromotion(options);
        else playHuman(options[0]);
        return;
      }
    }

    // otherwise select one of our own pieces, or clear the selection
    const ch = state.board[key];
    if (ch !== '.' && isWhitePiece(ch) === (state.turn === 0) && state.selected !== key) state.selected = key;
    else state.selected = null;
    state.animate = false;
    render();
  }

  function playHuman(m) {
    const promo = m.type === 'p' ? m.promo.charCodeAt(0) : 0;
    const before = state.board;
    const legalBefore = state.legal;
    if (!engine.play(m.fr, m.fc, m.tr, m.tc, promo)) return;
    afterPlayed(m, before, legalBefore);
    maybeAiMove();
  }

  // The engine has just played `move`; update everything we show
  function afterPlayed(move, before, legalBefore) {
    const mover = state.turn;
    syncFromEngine();
    state.history.push({ move, san: toSan(move, before, legalBefore, state.status), mover });
    state.lastMove = move;
    state.selected = null;
    state.hint = null;
    state.analysis = null;
    state.animate = true;
    if (state.mode === 'local' && settings.rotate) state.flipped = state.turn === 1;
    render();
    if (state.status >= 2) setTimeout(() => { if (state.inGame && state.status >= 2) showGameOver(); }, 750);
  }

  async function maybeAiMove() {
    if (state.mode !== 'ai' || !state.inGame || state.status >= 2 || state.turn === state.humanColor) return;
    const token = ++aiToken;
    state.busy = true;
    state.working = 'ai';
    render();
    await sleep(350);                       // lets the browser draw your move first
    if (token !== aiToken || !state.inGame) return;   // game was restarted or closed meanwhile
    const before = state.board;
    const legalBefore = state.legal;
    const reply = engine.ai(state.depth);
    state.busy = false;
    if (!reply) { render(); return; }
    afterPlayed(parseMove(reply), before, legalBefore);
  }

  /* ---------- hint and analysis ---------- */
  // Against the computer the hint is for you; with two players it is for whoever is to move.
  function canHint() {
    if (!state.inGame || state.busy || state.status >= 2 || !el.modal.hidden) return false;
    return state.mode === 'local' || state.turn === state.humanColor;
  }

  // "1.25" -> "+1.25"; a forced mate -> "+M3" (White mates) or "-M3" (Black mates)
  function formatScore(score, depth) {
    if (Math.abs(score) >= MATE - 1000) {
      const plies = depth - (Math.abs(score) - MATE);      // moves until the mate, counted in half-moves
      return (score > 0 ? '+' : '-') + 'M' + Math.max(1, Math.ceil(plies / 2));
    }
    const pawns = score / 100;
    return (pawns > 0 ? '+' : '') + pawns.toFixed(2);
  }

  function describeScore(score, depth) {
    if (Math.abs(score) >= MATE - 1000) {
      const n = Math.max(1, Math.ceil((depth - (Math.abs(score) - MATE)) / 2));
      return (score > 0 ? 'White' : 'Black') + ' can force mate in ' + n;
    }
    const a = Math.abs(score);
    if (a < 25) return 'Equal position';
    const side = score > 0 ? 'White' : 'Black';
    if (a < 100) return side + ' is slightly better';
    if (a < 300) return side + ' is better';
    return side + ' is winning';
  }

  // White's share of the evaluation bar, 0-100 (a smooth curve, like chess sites use)
  function whiteShare(score) {
    if (score >= MATE - 1000) return 100;
    if (score <= -(MATE - 1000)) return 0;
    return 100 / (1 + Math.exp(-score / 350));
  }

  // Ask the engine for its best moves for the side to move. They are shown, not played.
  async function showHint() {
    if (!canHint()) return;
    const length = state.history.length;
    state.busy = true;
    state.working = 'hint';
    state.selected = null;
    state.hint = null;
    state.analysis = null;
    state.animate = false;
    render();
    await sleep(60);                                   // lets "Finding a hint..." appear first
    if (!state.inGame || state.history.length !== length) { state.busy = false; return; }
    const depth = Math.max(2, state.depth);            // same strength as the setting, at least 2
    const text = engine.analyze(depth, 3);
    state.busy = false;
    if (text) {
      const moves = text.split(';').map((entry) => {
        const m = parseMove(entry);
        m.score = +entry.split(',')[6];
        m.san = toSan(m, state.board, state.legal, 0);
        return m;
      });
      state.hint = moves[0];
      state.analysis = { moves, depth };
    }
    render();
  }

  // The "Engine analysis" card and the evaluation bar
  function renderAnalysis() {
    const a = state.analysis;
    el.analysisEmpty.hidden = !!a;
    el.analysisBody.hidden = !a;
    el.analysisDepth.textContent = a ? '\u00b7 looks ' + a.depth + ' moves ahead' : '';
    el.evalBar.classList.toggle('idle', !a);
    el.evalBar.classList.toggle('flipped', state.flipped);

    if (!a) {
      el.analysisLines.textContent = '';
      el.evalWhite.style.height = '50%';
      el.evalBar.title = 'Evaluation of the position. Press Hint to analyse.';
      return;
    }
    const best = a.moves[0];
    el.analysisScore.textContent = formatScore(best.score, a.depth);
    el.analysisText.textContent = describeScore(best.score, a.depth);
    el.evalWhite.style.height = whiteShare(best.score).toFixed(1) + '%';
    el.evalBar.title = 'Evaluation: ' + formatScore(best.score, a.depth) + ' (positive = better for White)';

    el.analysisLines.textContent = '';
    a.moves.forEach((m, i) => {
      const li = document.createElement('li');
      const no = document.createElement('span');
      no.className = 'no';
      no.textContent = (i + 1) + '.';
      const san = document.createElement('span');
      san.textContent = m.san;
      const sc = document.createElement('span');
      sc.className = 'sc';
      sc.textContent = formatScore(m.score, a.depth);
      li.append(no, san, sc);
      el.analysisLines.appendChild(li);
    });
  }

  // One arrow per suggested move: the best one is strong, the others fainter
  function renderArrows() {
    const svg = el.arrows;
    while (svg.firstChild) svg.removeChild(svg.firstChild);
    if (!state.analysis) return;
    const NS = 'http://www.w3.org/2000/svg';
    const centre = (r, c) => [(state.flipped ? 7 - c : c) + 0.5, (state.flipped ? 7 - r : r) + 0.5];

    for (let i = state.analysis.moves.length - 1; i >= 0; i--) {   // draw the best one last, on top
      const m = state.analysis.moves[i];
      const [x1, y1] = centre(m.fr, m.fc);
      const [x2, y2] = centre(m.tr, m.tc);
      const dx = x2 - x1, dy = y2 - y1, len = Math.hypot(dx, dy);
      if (len === 0) continue;
      const ux = dx / len, uy = dy / len, px = -uy, py = ux;
      const sw = ARROW_WIDTH[i] / 2, hw = ARROW_WIDTH[i] * 1.9, head = 0.36;
      const sx = x1 + ux * 0.2, sy = y1 + uy * 0.2;                // start a little off the piece
      const tx = x2 - ux * 0.08, ty = y2 - uy * 0.08;              // tip
      const bx = tx - ux * head, by = ty - uy * head;              // where the head begins
      const pts = [[sx + px * sw, sy + py * sw], [bx + px * sw, by + py * sw], [bx + px * hw, by + py * hw],
                   [tx, ty], [bx - px * hw, by - py * hw], [bx - px * sw, by - py * sw], [sx - px * sw, sy - py * sw]];
      const poly = document.createElementNS(NS, 'polygon');
      poly.setAttribute('points', pts.map((p) => p[0].toFixed(3) + ',' + p[1].toFixed(3)).join(' '));
      poly.setAttribute('fill', 'rgb(70, 140, 255)');
      poly.setAttribute('fill-opacity', String(ARROW_OPACITY[i]));
      poly.setAttribute('data-rank', String(i + 1));
      svg.appendChild(poly);
    }
  }

  /* ---------- undo ---------- */
  // How many half-moves to take back (0 = not possible right now)
  function undoSteps() {
    if (!state.inGame || state.busy) return 0;
    if (state.mode === 'ai') {
      // go back to the human's previous turn
      const steps = state.turn === state.humanColor ? 2 : 1;
      const minLength = state.humanColor === 1 ? 1 : 0;   // the computer's opening move stays
      return state.history.length - steps >= minLength ? steps : 0;
    }
    return state.history.length > 0 ? 1 : 0;
  }

  function undo() {
    const steps = undoSteps();
    if (!steps) return;
    for (let i = 0; i < steps; i++) { engine.undo(); state.history.pop(); }
    syncFromEngine();
    const prev = state.history[state.history.length - 1];
    state.lastMove = prev ? prev.move : null;
    state.selected = null;
    state.hint = null;
    state.analysis = null;
    state.animate = false;
    if (state.mode === 'local' && settings.rotate) state.flipped = state.turn === 1;
    closeModal();
    render();
  }

  /* ============================================================
     Pop-ups: promotion and game over
     ============================================================ */
  function openModal(node) {
    el.modalCard.textContent = '';
    el.modalCard.appendChild(node);
    el.modal.hidden = false;
  }
  function closeModal() {
    el.modal.hidden = true;
    el.modalCard.textContent = '';
  }

  function askPromotion(options) {
    const color = state.turn;
    const wrap = document.createElement('div');
    const title = document.createElement('h3');
    title.textContent = 'Promote pawn to';
    const row = document.createElement('div');
    row.className = 'promo-row';
    for (const letter of ['q', 'r', 'b', 'n']) {
      const opt = options.find((m) => m.promo === letter);
      if (!opt) continue;
      const b = document.createElement('button');
      b.className = 'promo-btn ' + (color === 0 ? 'w' : 'b');
      b.textContent = GLYPH[letter];
      b.setAttribute('aria-label', PIECE_NAME[letter]);
      b.addEventListener('click', () => { closeModal(); playHuman(opt); });
      row.appendChild(b);
    }
    wrap.append(title, row);
    openModal(wrap);
  }

  function showGameOver() {
    let big;
    let text;
    if (state.status === 3) {
      big = 'Draw';
      text = 'Stalemate: the player to move has no legal move but is not in check.';
    } else if (state.mode === 'ai') {
      const humanWon = 1 - state.turn === state.humanColor;
      big = humanWon ? 'You win!' : 'You lose';
      text = humanWon ? 'Checkmate. Well played!' : 'Checkmate. The computer wins this one.';
    } else {
      big = (state.turn === 0 ? 'Black' : 'White') + ' wins';
      text = 'Checkmate.';
    }

    const wrap = document.createElement('div');
    const h = document.createElement('h3');
    h.textContent = 'Game over';
    const b = document.createElement('div');
    b.className = 'big';
    b.textContent = big;
    const p = document.createElement('p');
    p.textContent = text;
    const actions = document.createElement('div');
    actions.className = 'modal-actions';
    actions.append(
      makeButton('Play again', 'primary', () => { closeModal(); startGame(); }),
      makeButton('View the board', 'btn', closeModal),
      makeButton('Back to menu', 'btn', () => { closeModal(); show('home'); }),
    );
    wrap.append(h, b, p, actions);
    openModal(wrap);
  }

  function makeButton(label, cls, onClick) {
    const b = document.createElement('button');
    b.className = cls;
    b.textContent = label;
    b.addEventListener('click', onClick);
    return b;
  }

  /* ============================================================
     Screens
     ============================================================ */
  function show(name) {
    for (const s of $$('.screen')) s.classList.toggle('active', s.id === 'screen-' + name);
    if (name !== 'game') {
      state.inGame = false;
      state.busy = false;
      aiToken++;
      closeModal();
    }
    window.scrollTo(0, 0);
  }

  function openSetup(mode) {
    state.mode = mode;
    const ai = mode === 'ai';
    el.aiOptions.hidden = !ai;
    el.localOptions.hidden = ai;
    el.levelTitle.textContent = ai ? 'Difficulty' : 'Hint strength';
    el.setupSub.textContent = ai
      ? 'Playing the computer. Pick a look and how you want to play.'
      : 'Two players on one screen. Pick a look, and how hard the Hint button thinks.';
    applySettings();                                 // refreshes the strength text for this mode
    show('setup');
  }

  function startGame() {
    aiToken++;
    closeModal();
    engine.newGame();

    state.depth = LEVELS[settings.level].depth;
    state.levelName = LEVELS[settings.level].name;
    if (state.mode === 'ai') {
      const choice = settings.color === 'random' ? (Math.random() < 0.5 ? 'white' : 'black') : settings.color;
      state.humanColor = choice === 'white' ? 0 : 1;
      state.flipped = state.humanColor === 1;      // keep your pieces at the bottom
    } else {
      state.flipped = false;
    }

    state.history = [];
    state.lastMove = null;
    state.selected = null;
    state.hint = null;
    state.analysis = null;
    state.animate = false;
    state.busy = false;
    state.inGame = true;
    syncFromEngine();
    show('game');
    render();
    maybeAiMove();                                  // the computer opens if you chose Black
  }

  function leaveGame() {
    if (state.inGame && state.history.length > 0 && state.status < 2 &&
        !window.confirm('Leave this game?')) return false;
    show('home');
    return true;
  }

  /* ============================================================
     Setup screen controls
     ============================================================ */
  function buildThemeList() {
    el.themeList.textContent = '';
    for (const t of THEMES) {
      const b = document.createElement('button');
      b.className = 'theme';
      b.dataset.theme = t.id;
      b.setAttribute('role', 'radio');
      const mini = document.createElement('span');
      mini.className = 'mini';
      mini.dataset.theme = t.id;                    // gives the mini board this theme's colours
      for (let i = 0; i < 16; i++) mini.appendChild(document.createElement('i'));
      const name = document.createElement('span');
      name.className = 'theme-name';
      name.textContent = t.name;
      b.append(mini, name);
      b.addEventListener('click', () => { settings.theme = t.id; saveSettings(); applySettings(); });
      el.themeList.appendChild(b);
    }
  }

  function bindSegmented(container, key) {
    container.addEventListener('click', (e) => {
      const b = e.target.closest('button');
      if (!b) return;
      settings[key] = b.dataset.value;
      saveSettings();
      applySettings();
    });
  }

  function applySettings() {
    document.body.dataset.theme = settings.theme;
    document.body.classList.toggle('no-coords', !settings.coords);

    for (const b of $$('.theme')) b.setAttribute('aria-checked', String(b.dataset.theme === settings.theme));
    for (const b of el.segColor.querySelectorAll('button')) b.classList.toggle('on', b.dataset.value === settings.color);
    for (const b of el.segLevel.querySelectorAll('button')) b.classList.toggle('on', b.dataset.value === settings.level);
    el.optCoords.checked = settings.coords;
    el.optHints.checked = settings.hints;
    el.optRotate.checked = settings.rotate;
    el.levelHint.textContent = state.mode === 'ai'
      ? LEVELS[settings.level].hint
      : LEVELS[settings.level].hint.split('. ')[0] + '. The Hint button uses this strength.';

    const theme = THEMES.find((t) => t.id === settings.theme);
    el.previewCaption.textContent = theme.name + ' theme';
    paintPreview();
    if (state.inGame) render();
  }

  function bindSetup() {
    bindSegmented(el.segColor, 'color');
    bindSegmented(el.segLevel, 'level');
    el.optCoords.addEventListener('change', () => { settings.coords = el.optCoords.checked; saveSettings(); applySettings(); });
    el.optHints.addEventListener('change', () => { settings.hints = el.optHints.checked; saveSettings(); applySettings(); });
    el.optRotate.addEventListener('change', () => { settings.rotate = el.optRotate.checked; saveSettings(); applySettings(); });

    $('#btn-back-home').addEventListener('click', () => show('home'));
    $('#btn-start').addEventListener('click', startGame);
    for (const card of $$('.mode-card')) card.addEventListener('click', () => openSetup(card.dataset.mode));

    // game screen buttons
    el.btnUndo.addEventListener('click', undo);
    el.btnHint.addEventListener('click', showHint);
    $('#btn-flip').addEventListener('click', () => { state.flipped = !state.flipped; state.animate = false; render(); });
    $('#btn-new').addEventListener('click', () => {
      if (state.history.length > 0 && state.status < 2 && !window.confirm('Start a new game?')) return;
      startGame();
    });
    $('#btn-menu').addEventListener('click', leaveGame);

    el.board.addEventListener('click', onBoardClick);
    el.modal.addEventListener('click', (e) => { if (e.target === el.modal) closeModal(); });
    document.addEventListener('keydown', (e) => {
      if ((e.key === 'h' || e.key === 'H') && !e.ctrlKey && !e.metaKey && !e.altKey) { showHint(); return; }
      if (e.key !== 'Escape') return;
      if (!el.modal.hidden) closeModal();
      else if (state.selected !== null) { state.selected = null; state.animate = false; render(); }
    });
  }

  /* ============================================================
     Start-up
     ============================================================ */
  async function boot() {
    gameSquares = buildGrid(el.board);
    previewSquares = buildGrid(el.preview);
    buildThemeList();
    bindSetup();
    applySettings();

    try {
      engine = await loadEngine();
      el.note.textContent = 'Engine ready.';
      for (const card of $$('.mode-card')) card.disabled = false;
    } catch (err) {
      el.note.classList.add('error');
      el.note.textContent = 'Could not load the engine. Serve this folder with a web server ' +
        '(for example: python3 -m http.server) instead of opening the file directly.';
      console.error(err);
    }
  }

  boot();
})();
