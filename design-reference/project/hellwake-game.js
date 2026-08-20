// Hellwake — real-time vertical slice engine.
// Mount:  const api = await mount(containerEl, { onHud, onEvent })
// The approved composition (camera framing, grading, HUD) is the visual spec.

const V = '0.160.1';
const url = (p) => `https://esm.sh/three@${V}${p}?deps=three@${V}`;

export async function mount(container, hooks = {}) {
  const THREE = await import(`https://esm.sh/three@${V}`);
  let composer = null, bloom = null, RenderPassC = null;
  const emit = (k, v) => hooks.onEvent && hooks.onEvent(k, v);

  /* ---------- renderer ---------- */
  const renderer = new THREE.WebGLRenderer({ antialias: true, powerPreference: 'high-performance' });
  renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
  renderer.shadowMap.enabled = true;
  renderer.shadowMap.type = THREE.PCFSoftShadowMap;
  renderer.toneMapping = THREE.ACESFilmicToneMapping;
  renderer.toneMappingExposure = 1.05;
  renderer.outputColorSpace = THREE.SRGBColorSpace;
  Object.assign(renderer.domElement.style, { position: 'absolute', inset: '0', width: '100%', height: '100%', display: 'block' });
  container.appendChild(renderer.domElement);

  const scene = new THREE.Scene();
  scene.background = new THREE.Color(0x05070b);
  scene.fog = new THREE.FogExp2(0x0a0e14, 0.028);

  const camera = new THREE.PerspectiveCamera(34, 1, 0.5, 400);
  const CAM_OFF = new THREE.Vector3(0, 27, 25);

  /* ---------- world-space overlay (damage numbers, telegraph labels) ---------- */
  const overlay = document.createElement('div');
  Object.assign(overlay.style, { position: 'absolute', inset: '0', overflow: 'hidden', pointerEvents: 'none',
    fontFamily: "'Barlow Condensed',sans-serif" });
  container.appendChild(overlay);

  /* ---------- procedural textures ---------- */
  const tex = (w, h, draw, rep = 1) => {
    const c = document.createElement('canvas'); c.width = w; c.height = h;
    draw(c.getContext('2d'), w, h);
    const t = new THREE.CanvasTexture(c);
    t.wrapS = t.wrapT = THREE.RepeatWrapping; t.repeat.set(rep, rep);
    t.colorSpace = THREE.SRGBColorSpace;
    return t;
  };
  const noise = (ctx, w, h, a, lo, hi) => {
    const img = ctx.getImageData(0, 0, w, h), d = img.data;
    for (let i = 0; i < d.length; i += 4) {
      const n = lo + Math.random() * (hi - lo);
      d[i] = Math.min(255, d[i] * n); d[i + 1] = Math.min(255, d[i + 1] * n); d[i + 2] = Math.min(255, d[i + 2] * n);
      d[i + 3] = 255 * a;
    }
    ctx.putImageData(img, 0, 0);
  };
  const cobble = tex(512, 512, (c, w, h) => {
    c.fillStyle = '#1b1d22'; c.fillRect(0, 0, w, h);
    for (let y = 0; y < 16; y++) for (let x = 0; x < 16; x++) {
      const o = (y % 2) * 16, s = 32;
      const g = 26 + Math.random() * 26;
      c.fillStyle = `rgb(${g},${g + 2},${g + 6})`;
      c.beginPath();
      c.roundRect(x * s + o - 1 + Math.random() * 2, y * s + Math.random() * 2, s - 3, s - 3, 4);
      c.fill();
      c.strokeStyle = 'rgba(0,0,0,.55)'; c.lineWidth = 2; c.stroke();
    }
    noise(c, w, h, 0.22, 0.7, 1.3);
  }, 14);
  const stone = tex(256, 256, (c, w, h) => {
    c.fillStyle = '#20222a'; c.fillRect(0, 0, w, h);
    for (let y = 0; y < 8; y++) for (let x = 0; x < 4; x++) {
      const g = 28 + Math.random() * 22;
      c.fillStyle = `rgb(${g},${g + 1},${g + 5})`;
      c.fillRect(x * 64 + (y % 2 ? 32 : 0), y * 32, 62, 30);
    }
    noise(c, w, h, 0.3, 0.65, 1.35);
  }, 3);

  const MAT = {
    floor: new THREE.MeshStandardMaterial({ map: cobble, color: 0x8f949c, roughness: 0.82, metalness: 0.12 }),
    stone: new THREE.MeshStandardMaterial({ map: stone, color: 0x7f848d, roughness: 0.92 }),
    dark: new THREE.MeshStandardMaterial({ color: 0x14161c, roughness: 0.85 }),
    metal: new THREE.MeshStandardMaterial({ color: 0x23262d, roughness: 0.42, metalness: 0.85 }),
    leather: new THREE.MeshStandardMaterial({ color: 0x1c1a18, roughness: 0.78 }),
    cloth: new THREE.MeshStandardMaterial({ color: 0x171519, roughness: 0.95 }),
    bone: new THREE.MeshStandardMaterial({ color: 0x9c9382, roughness: 0.75 }),
    ember: new THREE.MeshStandardMaterial({ color: 0x2a1206, emissive: 0xff5a14, emissiveIntensity: 2.6, roughness: 0.5 }),
    soul: new THREE.MeshStandardMaterial({ color: 0x0c1a22, emissive: 0x58c8ea, emissiveIntensity: 2.0, roughness: 0.5 }),
    blade: new THREE.MeshStandardMaterial({ color: 0x9aa2ad, roughness: 0.22, metalness: 1 })
  };

  /* ---------- lighting ---------- */
  scene.add(new THREE.HemisphereLight(0x4a5b73, 0x0a0b0f, 0.5));
  const moon = new THREE.DirectionalLight(0x9fbbe0, 1.15);
  moon.position.set(-26, 40, -18); moon.castShadow = true;
  moon.shadow.mapSize.set(2048, 2048);
  const sc = moon.shadow.camera; sc.left = -46; sc.right = 46; sc.top = 46; sc.bottom = -46; sc.near = 1; sc.far = 140;
  moon.shadow.bias = -0.0008;
  scene.add(moon);
  const fillWarm = new THREE.PointLight(0xff7a28, 0, 40);
  scene.add(fillWarm);

  /* ---------- arena ---------- */
  const colliders = []; // {x,z,r}
  const props = new THREE.Group(); scene.add(props);
  const ARENA = 30; // half-extent of playable floor

  const floor = new THREE.Mesh(new THREE.PlaneGeometry(96, 96, 1, 1), MAT.floor);
  floor.rotation.x = -Math.PI / 2; floor.receiveShadow = true; scene.add(floor);

  const box = (w, h, d, x, y, z, mat = MAT.stone, ry = 0, shadow = true) => {
    const m = new THREE.Mesh(new THREE.BoxGeometry(w, h, d), mat);
    m.position.set(x, y, z); m.rotation.y = ry;
    m.castShadow = shadow; m.receiveShadow = true; props.add(m); return m;
  };
  const column = (x, z, h = 12, r = 1.1, broken = false) => {
    const g = new THREE.Group(); g.position.set(x, 0, z);
    const shaft = new THREE.Mesh(new THREE.CylinderGeometry(r * 0.86, r, broken ? h * 0.55 : h, 12), MAT.stone);
    shaft.position.y = (broken ? h * 0.55 : h) / 2; shaft.castShadow = shaft.receiveShadow = true; g.add(shaft);
    const base = new THREE.Mesh(new THREE.BoxGeometry(r * 2.8, 0.7, r * 2.8), MAT.stone);
    base.position.y = 0.35; base.castShadow = base.receiveShadow = true; g.add(base);
    if (!broken) {
      const cap = new THREE.Mesh(new THREE.BoxGeometry(r * 2.6, 0.9, r * 2.6), MAT.stone);
      cap.position.y = h; cap.castShadow = true; g.add(cap);
    }
    props.add(g); colliders.push({ x, z, r: r + 0.5 });
    return g;
  };
  const rubble = (x, z, n = 7, spread = 3) => {
    for (let i = 0; i < n; i++) {
      const s = 0.4 + Math.random() * 1.3;
      const m = box(s, s * 0.7, s * 0.9, x + (Math.random() - 0.5) * spread, s * 0.35, z + (Math.random() - 0.5) * spread,
        MAT.stone, Math.random() * 3);
      m.rotation.z = (Math.random() - 0.5) * 0.4;
    }
  };
  const grave = (x, z, r = 0) => {
    const g = new THREE.Group(); g.position.set(x, 0, z); g.rotation.set(0.06, r, (Math.random() - 0.5) * 0.14);
    const slab = new THREE.Mesh(new THREE.BoxGeometry(1.5, 2.4, 0.35), MAT.stone);
    slab.position.y = 1.2; slab.castShadow = slab.receiveShadow = true; g.add(slab);
    const top = new THREE.Mesh(new THREE.CylinderGeometry(0.75, 0.75, 0.35, 14, 1, false, 0, Math.PI), MAT.stone);
    top.rotation.set(Math.PI / 2, 0, 0); top.position.y = 2.4; top.castShadow = true; g.add(top);
    props.add(g); colliders.push({ x, z, r: 1.1 });
  };
  const bones = (x, z) => {
    for (let i = 0; i < 5; i++) {
      const m = new THREE.Mesh(new THREE.CapsuleGeometry(0.09, 0.7 + Math.random() * 0.6, 3, 6), MAT.bone);
      m.position.set(x + (Math.random() - 0.5) * 2.6, 0.12, z + (Math.random() - 0.5) * 2.6);
      m.rotation.set(Math.PI / 2, Math.random() * 3, Math.random() * 3);
      m.castShadow = true; props.add(m);
    }
  };

  // braziers: the warm key lights of the composition
  const braziers = [];
  const brazier = (x, z, scale = 1) => {
    const g = new THREE.Group(); g.position.set(x, 0, z);
    const bowl = new THREE.Mesh(new THREE.CylinderGeometry(0.85 * scale, 0.5 * scale, 0.8 * scale, 12), MAT.metal);
    bowl.position.y = 1.9 * scale; bowl.castShadow = true; g.add(bowl);
    const stem = new THREE.Mesh(new THREE.CylinderGeometry(0.16 * scale, 0.34 * scale, 1.9 * scale, 8), MAT.metal);
    stem.position.y = 0.95 * scale; stem.castShadow = true; g.add(stem);
    const flame = new THREE.Mesh(new THREE.IcosahedronGeometry(0.75 * scale, 1),
      new THREE.MeshStandardMaterial({ color: 0x3a1000, emissive: 0xff7a1e, emissiveIntensity: 3.4, roughness: 1 }));
    flame.position.y = 2.6 * scale; g.add(flame);
    const light = new THREE.PointLight(0xff6f22, 5.2 * scale, 34 * scale, 2);
    light.position.y = 2.9 * scale; g.add(light);
    props.add(g); colliders.push({ x, z, r: 0.9 * scale });
    braziers.push({ flame, light, base: 5.2 * scale, seed: Math.random() * 10 });
  };

  // ruined arcade left/right, cathedral facade upstage, back wall of rubble
  for (let i = 0; i < 7; i++) {
    column(-22 - (i % 2) * 1.5, -18 + i * 6, 11 - (i % 3), 1.15, i === 3 || i === 5);
    column(22 + (i % 2) * 1.5, -18 + i * 6, 12 - (i % 2) * 3, 1.15, i === 2);
  }
  for (let i = 0; i < 6; i++) { // architrave fragments
    if (i !== 2) box(4.6, 1.1, 2.2, -22, 11.4, -15 + i * 6, MAT.stone);
    if (i !== 4) box(4.6, 1.1, 2.2, 22, 12.2, -15 + i * 6, MAT.stone);
  }
  box(46, 26, 3, 0, 13, -34, MAT.stone);                       // cathedral wall
  for (let i = -2; i <= 2; i++) {                              // gothic window recesses
    box(4.4, 11, 1.2, i * 8.5, 9, -32.2, MAT.dark);
    const arch = new THREE.Mesh(new THREE.CylinderGeometry(2.2, 2.2, 1.2, 16, 1, false, 0, Math.PI), MAT.dark);
    arch.rotation.set(Math.PI / 2, 0, 0); arch.position.set(i * 8.5, 14.5, -32.2); props.add(arch);
  }
  [-16, 0, 16].forEach((x, i) => {                             // spires
    const sp = new THREE.Mesh(new THREE.ConeGeometry(2.6 - i % 2, 16 + (i === 1 ? 10 : 0), 6), MAT.stone);
    sp.position.set(x, 26 + (i === 1 ? 13 : 0) / 2 + 8, -36); sp.castShadow = true; props.add(sp);
  });
  box(70, 5, 4, 0, 2.5, 30, MAT.stone);                        // downstage ruin wall (frames the camera)
  box(16, 9, 4, -27, 4.5, 24, MAT.stone, 0.3);
  box(16, 9, 4, 27, 4.5, 24, MAT.stone, -0.3);
  colliders.push({ x: 0, z: 31, r: 6 }, { x: 0, z: -33, r: 8 });

  // steps up to the cathedral (the Gravewarden's ground)
  for (let i = 0; i < 4; i++) box(30 - i * 2, 0.6, 2.2, 0, 0.3 + i * 0.55, -24 + i * 2.2, MAT.stone);

  brazier(-13, 6, 1.15); brazier(13, 6, 1.15); brazier(-9, -16, 1); brazier(9, -16, 1); brazier(0, 18, 0.85);
  [[-18, 14], [18, 15], [-20, -6], [20, -4], [-6, 22], [8, 24]].forEach(([x, z]) => rubble(x, z, 8, 4));
  [[-16, 20, 0.2], [-11, 22, -0.4], [15, 20, 0.3], [10, 23, 0.1], [-19, 2, 0.5], [19, 8, -0.5]].forEach(([x, z, r]) => grave(x, z, r));
  [[-8, 12], [6, 16], [-2, -8], [14, -12]].forEach(([x, z]) => bones(x, z));
  // fallen banners
  [[-21, 18, 0.4], [21, 12, -0.35]].forEach(([x, z, r]) => {
    const b = box(3.4, 6, 0.12, x, 5, z, new THREE.MeshStandardMaterial({ color: 0x4b1512, roughness: 0.95 }), r);
    b.rotation.z = 0.12;
  });

  /* ---------- ambient ash ---------- */
  const ashN = 900, ashPos = new Float32Array(ashN * 3), ashVel = new Float32Array(ashN);
  for (let i = 0; i < ashN; i++) {
    ashPos[i * 3] = (Math.random() - 0.5) * 80; ashPos[i * 3 + 1] = Math.random() * 26; ashPos[i * 3 + 2] = (Math.random() - 0.5) * 80;
    ashVel[i] = 0.6 + Math.random() * 1.9;
  }
  const ashGeo = new THREE.BufferGeometry();
  ashGeo.setAttribute('position', new THREE.BufferAttribute(ashPos, 3));
  const ash = new THREE.Points(ashGeo, new THREE.PointsMaterial({ color: 0xffa257, size: 0.13, transparent: true, opacity: 0.75, depthWrite: false, blending: THREE.AdditiveBlending }));
  scene.add(ash);

  /* ---------- character factory ---------- */
  const rimLight = (color, intensity, dist) => new THREE.PointLight(color, intensity, dist, 2);

  function makeHero() {
    const g = new THREE.Group();
    const torso = new THREE.Mesh(new THREE.CapsuleGeometry(0.42, 0.7, 4, 12), MAT.leather);
    torso.position.y = 1.42; torso.castShadow = true; g.add(torso);
    const chest = new THREE.Mesh(new THREE.BoxGeometry(0.95, 0.6, 0.55), MAT.metal);
    chest.position.y = 1.62; chest.castShadow = true; g.add(chest);
    const hood = new THREE.Mesh(new THREE.ConeGeometry(0.4, 0.62, 8), MAT.cloth);
    hood.position.y = 2.22; hood.castShadow = true; g.add(hood);
    const head = new THREE.Mesh(new THREE.SphereGeometry(0.24, 14, 12), MAT.leather);
    head.position.set(0, 2.06, 0.07); g.add(head);
    const cloak = new THREE.Mesh(new THREE.ConeGeometry(0.72, 1.7, 10, 1, true), MAT.cloth);
    cloak.position.set(0, 1.25, -0.16); cloak.castShadow = true; g.add(cloak);
    const legs = [];
    for (const s of [-1, 1]) {
      const l = new THREE.Mesh(new THREE.CapsuleGeometry(0.16, 0.62, 3, 8), MAT.leather);
      l.position.set(0.2 * s, 0.62, 0); l.castShadow = true; g.add(l); legs.push(l);
    }
    const armR = new THREE.Group(); armR.position.set(-0.5, 1.68, 0); g.add(armR);
    const upper = new THREE.Mesh(new THREE.CapsuleGeometry(0.14, 0.72, 3, 8), MAT.ember.clone());
    upper.material.emissiveIntensity = 1.5; upper.position.y = -0.3; armR.add(upper);
    const armL = new THREE.Group(); armL.position.set(0.5, 1.68, 0); g.add(armL);
    const upperL = new THREE.Mesh(new THREE.CapsuleGeometry(0.14, 0.72, 3, 8), MAT.leather);
    upperL.position.y = -0.3; armL.add(upperL);
    const weapon = new THREE.Group(); weapon.position.set(0, -0.62, 0); armR.add(weapon);
    const hilt = new THREE.Mesh(new THREE.CylinderGeometry(0.06, 0.06, 0.4, 6), MAT.metal); weapon.add(hilt);
    const blade = new THREE.Mesh(new THREE.BoxGeometry(0.12, 1.9, 0.03), MAT.blade);
    blade.position.y = 1.05; blade.castShadow = true; weapon.add(blade);
    const glyph = new THREE.Mesh(new THREE.BoxGeometry(0.04, 1.4, 0.045), MAT.ember);
    glyph.position.y = 1.0; weapon.add(glyph);
    const rim = rimLight(0xffa457, 2.4, 7); rim.position.set(0, 2.1, 0.9); g.add(rim);
    const trail = new THREE.Mesh(new THREE.RingGeometry(1.1, 2.9, 24, 1, -0.9, 1.9),
      new THREE.MeshBasicMaterial({ color: 0xffc98a, transparent: true, opacity: 0, side: THREE.DoubleSide, blending: THREE.AdditiveBlending, depthWrite: false }));
    trail.rotation.x = -Math.PI / 2; trail.position.y = 1.2; g.add(trail);
    g.userData = { armR, armL, legs, weapon, trail, rim, cloak };
    return g;
  }

  function makeReaver() { // armored melee pressure
    const g = new THREE.Group();
    const torso = new THREE.Mesh(new THREE.BoxGeometry(1.15, 1.25, 0.75), MAT.metal);
    torso.position.y = 1.6; torso.castShadow = true; g.add(torso);
    const core = new THREE.Mesh(new THREE.BoxGeometry(0.55, 0.5, 0.1), MAT.ember);
    core.position.set(0, 1.5, 0.4); g.add(core);
    const helm = new THREE.Mesh(new THREE.ConeGeometry(0.36, 0.68, 6), MAT.metal);
    helm.position.y = 2.5; helm.castShadow = true; g.add(helm);
    for (const s of [-1, 1]) {
      const horn = new THREE.Mesh(new THREE.ConeGeometry(0.08, 0.85, 5), MAT.metal);
      horn.position.set(0.22 * s, 2.75, -0.05); horn.rotation.z = 0.5 * s; horn.rotation.x = -0.3; g.add(horn);
      const pad = new THREE.Mesh(new THREE.SphereGeometry(0.34, 10, 8, 0, Math.PI * 2, 0, Math.PI / 2), MAT.metal);
      pad.position.set(0.72 * s, 2.05, 0); pad.castShadow = true; g.add(pad);
      const leg = new THREE.Mesh(new THREE.CapsuleGeometry(0.19, 0.75, 3, 8), MAT.metal);
      leg.position.set(0.28 * s, 0.7, 0); leg.castShadow = true; g.add(leg);
    }
    const eyes = new THREE.Mesh(new THREE.BoxGeometry(0.3, 0.05, 0.05), MAT.ember);
    eyes.position.set(0, 2.42, 0.3); g.add(eyes);
    const armR = new THREE.Group(); armR.position.set(-0.7, 2.0, 0); g.add(armR);
    const arm = new THREE.Mesh(new THREE.CapsuleGeometry(0.16, 0.8, 3, 8), MAT.metal); arm.position.y = -0.35; armR.add(arm);
    const cleaver = new THREE.Mesh(new THREE.BoxGeometry(0.55, 1.5, 0.09), MAT.blade);
    cleaver.position.set(0, -1.2, 0.1); cleaver.castShadow = true; armR.add(cleaver);
    const rim = rimLight(0xff6a22, 1.5, 6); rim.position.y = 1.8; g.add(rim);
    g.userData = { armR, rim, core };
    return g;
  }

  function makeWraith() { // fast flanker
    const g = new THREE.Group();
    const body = new THREE.Mesh(new THREE.ConeGeometry(0.62, 2.5, 10, 1, true),
      new THREE.MeshStandardMaterial({ color: 0x0d1218, emissive: 0x1d5c74, emissiveIntensity: 0.7, transparent: true, opacity: 0.82, roughness: 1 }));
    body.position.y = 1.5; body.castShadow = true; g.add(body);
    const head = new THREE.Mesh(new THREE.SphereGeometry(0.3, 12, 10), MAT.bone);
    head.position.y = 2.75; head.castShadow = true; g.add(head);
    const eyes = new THREE.Mesh(new THREE.BoxGeometry(0.26, 0.05, 0.05), MAT.soul);
    eyes.position.set(0, 2.78, 0.26); g.add(eyes);
    for (const s of [-1, 1]) {
      const claw = new THREE.Mesh(new THREE.ConeGeometry(0.09, 1.1, 5), MAT.bone);
      claw.position.set(0.6 * s, 1.9, 0.2); claw.rotation.x = 2.4; claw.castShadow = true; g.add(claw);
    }
    const rim = rimLight(0x63cdec, 1.6, 7); rim.position.y = 2.2; g.add(rim);
    g.userData = { rim, float: Math.random() * 6 };
    return g;
  }

  function makeAcolyte() { // ranged caster
    const g = new THREE.Group();
    const robe = new THREE.Mesh(new THREE.ConeGeometry(0.72, 2.3, 12), MAT.cloth);
    robe.position.y = 1.15; robe.castShadow = true; g.add(robe);
    const hood = new THREE.Mesh(new THREE.ConeGeometry(0.34, 0.7, 8), MAT.cloth);
    hood.position.y = 2.5; hood.castShadow = true; g.add(hood);
    const face = new THREE.Mesh(new THREE.SphereGeometry(0.2, 12, 10), MAT.bone);
    face.position.set(0, 2.32, 0.1); g.add(face);
    const staff = new THREE.Mesh(new THREE.CylinderGeometry(0.06, 0.06, 3, 6), MAT.leather);
    staff.position.set(0.62, 1.5, 0); staff.castShadow = true; g.add(staff);
    const orb = new THREE.Mesh(new THREE.IcosahedronGeometry(0.24, 1), MAT.ember);
    orb.position.set(0.62, 3.05, 0); g.add(orb);
    const light = rimLight(0xff7a2a, 3, 12); light.position.set(0.62, 3.05, 0); g.add(light);
    g.userData = { orb, rim: light };
    return g;
  }

  function makeWarden() { // the Gravewarden — elite
    const g = new THREE.Group();
    const s = 1.9;
    const torso = new THREE.Mesh(new THREE.BoxGeometry(1.5 * s, 1.7 * s, 0.95 * s), MAT.metal);
    torso.position.y = 2.5 * s; torso.castShadow = true; g.add(torso);
    const skirt = new THREE.Mesh(new THREE.ConeGeometry(1.35 * s, 2.2 * s, 10, 1, true), MAT.cloth);
    skirt.position.y = 1.5 * s; skirt.castShadow = true; g.add(skirt);
    const core = new THREE.Mesh(new THREE.BoxGeometry(0.8 * s, 0.7 * s, 0.12 * s), MAT.ember);
    core.position.set(0, 2.4 * s, 0.5 * s); g.add(core);
    const helm = new THREE.Mesh(new THREE.ConeGeometry(0.5 * s, 0.95 * s, 6), MAT.metal);
    helm.position.y = 3.85 * s; helm.castShadow = true; g.add(helm);
    const eyes = new THREE.Mesh(new THREE.BoxGeometry(0.42 * s, 0.07 * s, 0.06 * s), MAT.ember);
    eyes.position.set(0, 3.72 * s, 0.42 * s); g.add(eyes);
    for (const sd of [-1, 1]) {
      const horn = new THREE.Mesh(new THREE.ConeGeometry(0.13 * s, 1.5 * s, 6), MAT.metal);
      horn.position.set(0.34 * s * sd, 4.3 * s, -0.1 * s); horn.rotation.z = 0.62 * sd; horn.rotation.x = -0.35;
      horn.castShadow = true; g.add(horn);
      const pad = new THREE.Mesh(new THREE.SphereGeometry(0.55 * s, 12, 8, 0, Math.PI * 2, 0, Math.PI / 2), MAT.metal);
      pad.position.set(1.02 * s * sd, 3.15 * s, 0); pad.castShadow = true; g.add(pad);
      const leg = new THREE.Mesh(new THREE.CapsuleGeometry(0.27 * s, 1.1 * s, 4, 8), MAT.metal);
      leg.position.set(0.42 * s * sd, 1.0 * s, 0); leg.castShadow = true; g.add(leg);
    }
    const armR = new THREE.Group(); armR.position.set(-1.0 * s, 3.1 * s, 0); g.add(armR);
    const arm = new THREE.Mesh(new THREE.CapsuleGeometry(0.24 * s, 1.2 * s, 4, 8), MAT.metal);
    arm.position.y = -0.55 * s; arm.castShadow = true; armR.add(arm);
    const axe = new THREE.Group(); axe.position.y = -1.5 * s; armR.add(axe);
    const shaft = new THREE.Mesh(new THREE.CylinderGeometry(0.1 * s, 0.1 * s, 3.4 * s, 8), MAT.leather);
    shaft.castShadow = true; axe.add(shaft);
    const head = new THREE.Mesh(new THREE.BoxGeometry(1.5 * s, 1.1 * s, 0.16 * s), MAT.blade);
    head.position.set(0.5 * s, 1.5 * s, 0); head.castShadow = true; axe.add(head);
    const glyph = new THREE.Mesh(new THREE.TorusGeometry(0.34 * s, 0.07 * s, 8, 16), MAT.ember);
    glyph.position.set(0.6 * s, 1.5 * s, 0.02 * s); axe.add(glyph);
    const rim = rimLight(0xff5a18, 6, 20); rim.position.y = 3 * s; g.add(rim);
    const aura = new THREE.Mesh(new THREE.RingGeometry(2.2, 9, 40),
      new THREE.MeshBasicMaterial({ color: 0xff5a1e, transparent: true, opacity: 0, side: THREE.DoubleSide, blending: THREE.AdditiveBlending, depthWrite: false }));
    aura.rotation.x = -Math.PI / 2; aura.position.y = 0.06; g.add(aura);
    g.userData = { armR, axe, rim, aura, core };
    return g;
  }

  /* ---------- telegraphs, VFX pools ---------- */
  const decalMat = () => new THREE.MeshBasicMaterial({ color: 0xff6a20, transparent: true, opacity: 0.5, side: THREE.DoubleSide, depthWrite: false, blending: THREE.AdditiveBlending });
  const telegraphs = [];
  function telegraph(x, z, r, time, cb, color = 0xff6a20) {
    const ring = new THREE.Mesh(new THREE.RingGeometry(r * 0.94, r, 48), decalMat());
    ring.material.color.setHex(color); ring.rotation.x = -Math.PI / 2; ring.position.set(x, 0.07, z);
    const fill = new THREE.Mesh(new THREE.CircleGeometry(r, 40), decalMat());
    fill.material.color.setHex(color); fill.material.opacity = 0.14; fill.rotation.x = -Math.PI / 2; fill.position.set(x, 0.06, z);
    scene.add(ring, fill);
    telegraphs.push({ ring, fill, t: 0, time, r, cb, x, z });
  }
  const bursts = [];
  function burst(x, y, z, color = 0xffb060, scale = 1, count = 18) {
    const geo = new THREE.BufferGeometry(), pos = new Float32Array(count * 3), vel = [];
    for (let i = 0; i < count; i++) {
      pos[i * 3] = x; pos[i * 3 + 1] = y; pos[i * 3 + 2] = z;
      const a = Math.random() * Math.PI * 2, sp = (2 + Math.random() * 7) * scale;
      vel.push(new THREE.Vector3(Math.cos(a) * sp, (1.5 + Math.random() * 5) * scale, Math.sin(a) * sp));
    }
    geo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
    const p = new THREE.Points(geo, new THREE.PointsMaterial({ color, size: 0.22 * scale, transparent: true, opacity: 1, depthWrite: false, blending: THREE.AdditiveBlending }));
    scene.add(p); bursts.push({ p, vel, t: 0, life: 0.75 });
    const flash = new THREE.PointLight(color, 9 * scale, 16 * scale, 2);
    flash.position.set(x, y + 0.4, z); scene.add(flash);
    bursts.push({ light: flash, t: 0, life: 0.22 });
  }
  const shockwaves = [];
  function shockwave(x, z, rMax, color = 0xff7a2a) {
    const m = new THREE.Mesh(new THREE.RingGeometry(0.6, 1.1, 40), decalMat());
    m.material.color.setHex(color); m.rotation.x = -Math.PI / 2; m.position.set(x, 0.1, z);
    scene.add(m); shockwaves.push({ m, t: 0, life: 0.55, rMax });
  }

  /* ---------- floating numbers ---------- */
  const floaters = [];
  function damageNumber(pos, amount, kind) {
    const el = document.createElement('div');
    const crit = kind === 'crit';
    el.textContent = crit ? Math.round(amount).toLocaleString() : Math.round(amount).toLocaleString();
    Object.assign(el.style, {
      position: 'absolute', fontWeight: '600', whiteSpace: 'nowrap', transform: 'translate(-50%,-50%)',
      fontSize: (kind === 'player' ? 26 : crit ? 40 : 24) + 'px',
      color: kind === 'player' ? '#ff6a5a' : crit ? '#ffd6a6' : '#d8d2c7',
      textShadow: '0 2px 3px rgba(0,0,0,.95), 0 0 18px ' + (crit ? 'rgba(255,140,40,.85)' : 'rgba(0,0,0,.6)')
    });
    overlay.appendChild(el);
    floaters.push({ el, pos: pos.clone(), t: 0, life: 1.0, drift: (Math.random() - 0.5) * 0.8 });
  }

  /* ---------- entities ---------- */
  const hero = makeHero(); hero.position.set(0, 0, 18); scene.add(hero);
  const P = {
    obj: hero, hp: 100, hpMax: 100, wrath: 100, wrathMax: 100, xp: 0, level: 42,
    vel: new THREE.Vector3(), speed: 9.4, facing: 0, iframe: 0, dodgeCd: 0, atkCd: 0, swing: 0, walk: 0,
    statuses: [], bulwark: 0, dead: false
  };
  const cds = { LMB: 0, Q: 0, W: 0, E: 0, R: 0 };
  const CD = { LMB: 0.42, Q: 6, W: 12, E: 9, R: 40 };
  const COST = { Q: 18, W: 12, E: 22, R: 0 };

  const enemies = [];
  const ROLE = {
    reaver: { name: 'Ashbound Reaver', hp: 180, speed: 4.4, dmg: 11, range: 2.9, atkCd: 1.7, make: makeReaver, xp: 40, drop: 0.35, ring: 3.2 },
    wraith: { name: 'Cinder Wraith', hp: 110, speed: 7.2, dmg: 7, range: 2.4, atkCd: 1.1, make: makeWraith, xp: 32, drop: 0.3, ring: 2.6 },
    acolyte: { name: 'Pyre Acolyte', hp: 130, speed: 3.4, dmg: 14, range: 17, atkCd: 2.6, make: makeAcolyte, xp: 46, drop: 0.45, ring: 13 },
    warden: { name: 'Gravewarden of the Ninth Seal', hp: 3200, speed: 4.0, dmg: 26, range: 5.4, atkCd: 2.4, make: makeWarden, xp: 900, drop: 1, ring: 5, elite: true }
  };

  function spawn(role, x, z) {
    const def = ROLE[role], obj = def.make();
    obj.position.set(x, 0, z);
    scene.add(obj);
    const e = { role, def, obj, hp: def.hp, hpMax: def.hp, cd: 0.6 + Math.random(), stagger: 0, dead: false,
      slot: Math.random() * Math.PI * 2, anim: Math.random() * 10, phase: 1, casting: 0, telegraphed: 0 };
    enemies.push(e); return e;
  }

  /* ---------- loot ---------- */
  const RARITY = {
    common: { c: 0xb9bec6, label: 'Ashen Fragment', sub: 'COMMON' },
    magic: { c: 0x6fb6e8, label: 'Warded Sigil', sub: 'MAGIC' },
    rare: { c: 0xf0d070, label: 'Cinderforged Plate', sub: 'RARE' },
    legendary: { c: 0xff9a44, label: 'ASHFALL, THE LAST VOW', sub: 'ANCIENT GREATAXE · 812' }
  };
  const loot = [];
  function dropLoot(x, z, rarity) {
    const r = RARITY[rarity];
    const g = new THREE.Group(); g.position.set(x, 0, z);
    const item = new THREE.Mesh(new THREE.OctahedronGeometry(0.34, 0),
      new THREE.MeshStandardMaterial({ color: 0x101216, emissive: r.c, emissiveIntensity: 2.4, roughness: 0.4, metalness: 0.7 }));
    item.position.y = 0.7; item.castShadow = true; g.add(item);
    const beam = new THREE.Mesh(new THREE.CylinderGeometry(0.36, 0.5, 7, 12, 1, true),
      new THREE.MeshBasicMaterial({ color: r.c, transparent: true, opacity: 0.22, blending: THREE.AdditiveBlending, depthWrite: false, side: THREE.DoubleSide }));
    beam.position.y = 3.5; g.add(beam);
    const pool = new THREE.Mesh(new THREE.CircleGeometry(1.1, 24),
      new THREE.MeshBasicMaterial({ color: r.c, transparent: true, opacity: 0.3, blending: THREE.AdditiveBlending, depthWrite: false }));
    pool.rotation.x = -Math.PI / 2; pool.position.y = 0.05; g.add(pool);
    const light = new THREE.PointLight(r.c, 2.6, 10, 2); light.position.y = 1.2; g.add(light);
    scene.add(g);
    loot.push({ obj: g, item, rarity, def: r, t: 0 });
  }

  /* ---------- encounter flow ---------- */
  const STAGES = [
    { id: 'enter', text: 'Enter the Vaunhold plaza', hint: 'WASD to advance' },
    { id: 'wave1', text: 'Break the first procession', hint: '' },
    { id: 'advance', text: 'Advance to the cathedral steps', hint: '' },
    { id: 'wave2', text: 'Clear the ritual guard', hint: '' },
    { id: 'intro', text: 'The Gravewarden wakes', hint: '' },
    { id: 'boss', text: 'Slay the Gravewarden', hint: '' },
    { id: 'reward', text: 'Claim the Last Vow', hint: '' },
    { id: 'done', text: 'Vertical slice complete', hint: '' }
  ];
  let stageIdx = 0, stageTimer = 0, warden = null, cine = 0, bannerText = '', bannerT = 0;
  const stage = () => STAGES[stageIdx].id;
  function setStage(i) {
    stageIdx = i; stageTimer = 0;
    const s = STAGES[i];
    banner(s.text.toUpperCase());
    if (s.id === 'wave1') {
      spawn('reaver', -5, 6); spawn('reaver', 5, 7); spawn('wraith', 0, 2); spawn('acolyte', -12, -2);
    } else if (s.id === 'wave2') {
      spawn('reaver', -8, -8); spawn('reaver', 9, -7); spawn('wraith', -3, -12); spawn('wraith', 6, -13); spawn('acolyte', 13, -14);
    } else if (s.id === 'intro') {
      warden = spawn('warden', 0, -22); cine = 3.2;
    } else if (s.id === 'reward') {
      dropLoot(warden ? warden.obj.position.x : 0, warden ? warden.obj.position.z + 3 : -18, 'legendary');
    }
    emit('stage', s.id);
  }
  function banner(text) { bannerText = text; bannerT = 3; }

  /* ---------- input ---------- */
  const keys = new Set();
  const onDown = (e) => {
    const k = e.key.toLowerCase();
    keys.add(k);
    if (['q', 'w', 'e', 'r', ' ', 'shift'].includes(k) && e.target === document.body) e.preventDefault();
    if (k === 'q') ability('Q'); else if (k === 'e') ability('E'); else if (k === 'r') ability('R');
    else if (k === 'f') ability('W');
    else if (k === ' ' || k === 'shift') dodge();
  };
  const onUp = (e) => keys.delete(e.key.toLowerCase());
  const onPointer = (e) => { if (e.button === 0) attack(false); else attack(true); };
  const onCtx = (e) => e.preventDefault();
  window.addEventListener('keydown', onDown); window.addEventListener('keyup', onUp);
  renderer.domElement.addEventListener('pointerdown', onPointer);
  renderer.domElement.addEventListener('contextmenu', onCtx);

  /* ---------- combat ---------- */
  let shake = 0, hitStop = 0, camZoom = 1;
  const tmp = new THREE.Vector3();

  function damageEnemy(e, amount, kind = 'hit', knock = 0) {
    if (e.dead) return;
    e.hp -= amount;
    e.stagger = Math.max(e.stagger, kind === 'crit' ? 0.5 : 0.26);
    damageNumber(tmp.copy(e.obj.position).add(new THREE.Vector3(0, e.role === 'warden' ? 7.5 : 2.9, 0)), amount, kind);
    burst(e.obj.position.x, 1.4, e.obj.position.z, kind === 'crit' ? 0xffd39a : 0xff8a3c, e.role === 'warden' ? 1.4 : 0.9, 14);
    if (knock) {
      const d = tmp.copy(e.obj.position).sub(hero.position).setY(0).normalize().multiplyScalar(knock * (e.def.elite ? 0.18 : 1));
      e.obj.position.add(d);
    }
    if (e.hp <= 0) killEnemy(e);
  }
  function killEnemy(e) {
    e.dead = true; e.hp = 0;
    burst(e.obj.position.x, 1.5, e.obj.position.z, e.role === 'wraith' ? 0x63cdec : 0xff7a2a, e.def.elite ? 3 : 1.3, e.def.elite ? 60 : 26);
    shockwave(e.obj.position.x, e.obj.position.z, e.def.elite ? 14 : 3.4, e.role === 'wraith' ? 0x63cdec : 0xff7a2a);
    P.xp = Math.min(100, P.xp + e.def.xp / 12);
    if (!e.def.elite && Math.random() < e.def.drop) {
      const roll = Math.random();
      dropLoot(e.obj.position.x, e.obj.position.z, roll > 0.93 ? 'rare' : roll > 0.6 ? 'magic' : 'common');
    }
    shake = Math.max(shake, e.def.elite ? 1 : 0.25);
    if (e.def.elite) { banner('THE NINTH SEAL BREAKS'); cine = 3.4; }
    setTimeout(() => { scene.remove(e.obj); }, e.def.elite ? 2600 : 900);
  }
  function damagePlayer(amount) {
    if (P.iframe > 0 || P.dead) return;
    const mult = P.bulwark > 0 ? 0.45 : 1;
    P.hp = Math.max(0, P.hp - amount * mult);
    damageNumber(tmp.copy(hero.position).add(new THREE.Vector3(0, 3.1, 0)), amount * mult, 'player');
    shake = Math.max(shake, 0.35); hitStop = Math.max(hitStop, 0.05);
    if (P.hp <= 0) { P.dead = true; banner('YOU FELL — RESPAWNING'); setTimeout(respawn, 1800); }
  }
  function respawn() {
    P.dead = false; P.hp = P.hpMax; P.wrath = P.wrathMax;
    hero.position.set(0, 0, 18);
  }

  function attack(heavy) {
    if (P.dead || cine > 0) return;
    if (cds.LMB > 0) return;
    cds.LMB = heavy ? CD.LMB * 2.2 : CD.LMB;
    P.swing = heavy ? 0.42 : 0.26;
    const reach = heavy ? 4.4 : 3.4, arc = heavy ? 1.5 : 1.1, base = heavy ? 260 : 120;
    let hitAny = false;
    for (const e of enemies) {
      if (e.dead) continue;
      const d = tmp.copy(e.obj.position).sub(hero.position); d.y = 0;
      const dist = d.length();
      if (dist > reach + e.def.ring * 0.4) continue;
      const ang = Math.abs(angDiff(Math.atan2(d.x, d.z), P.facing));
      if (ang > arc) continue;
      const crit = Math.random() < 0.27;
      damageEnemy(e, base * (0.85 + Math.random() * 0.3) * (crit ? 2.6 : 1), crit ? 'crit' : 'hit', heavy ? 1.1 : 0.5);
      hitAny = true;
    }
    hero.userData.trail.material.opacity = heavy ? 0.85 : 0.55;
    if (hitAny) {
      hitStop = heavy ? 0.09 : 0.05; shake = Math.max(shake, heavy ? 0.5 : 0.22);
      P.wrath = Math.min(P.wrathMax, P.wrath + (heavy ? 9 : 5));
    }
  }
  function dodge() {
    if (P.dead || P.dodgeCd > 0 || cine > 0) return;
    P.dodgeCd = 1.1; P.iframe = 0.34;
    const dir = moveDir();
    if (dir.lengthSq() < 0.01) dir.set(Math.sin(P.facing), 0, Math.cos(P.facing));
    P.vel.copy(dir).multiplyScalar(26);
    burst(hero.position.x, 0.4, hero.position.z, 0xaab4c4, 0.5, 10);
  }
  function ability(key) {
    if (P.dead || cine > 0 || cds[key] > 0) return;
    if ((COST[key] || 0) > P.wrath) return;
    cds[key] = CD[key]; P.wrath -= COST[key] || 0;
    const fx = new THREE.Vector3(hero.position.x + Math.sin(P.facing) * 5, 0, hero.position.z + Math.cos(P.facing) * 5);
    if (key === 'Q') { // Emberbrand — forward cone
      P.swing = 0.4;
      shockwave(fx.x, fx.z, 7, 0xffa040);
      burst(fx.x, 1.2, fx.z, 0xffb060, 1.5, 30);
      for (const e of enemies) if (!e.dead && e.obj.position.distanceTo(fx) < 7.5) damageEnemy(e, 420 + Math.random() * 180, 'crit', 1.4);
      shake = Math.max(shake, 0.5); addStatus('EMBER 9s', '#ffab5e', 9);
    } else if (key === 'W') { // Ashen Bulwark
      P.bulwark = 10; addStatus('FORT 10s', '#9fd6ea', 10);
      shockwave(hero.position.x, hero.position.z, 4.5, 0x8ec8e6);
    } else if (key === 'E') { // Ruinfall — delayed AoE
      const px = fx.x, pz = fx.z;
      telegraph(px, pz, 5.5, 0.75, () => {
        burst(px, 1, pz, 0xb79bf0, 2, 34); shockwave(px, pz, 6.5, 0xb79bf0);
        for (const e of enemies) if (!e.dead && Math.hypot(e.obj.position.x - px, e.obj.position.z - pz) < 6.5)
          damageEnemy(e, 620 + Math.random() * 220, 'crit', 1.8);
        shake = Math.max(shake, 0.7);
      }, 0xb79bf0);
    } else if (key === 'R') { // Wake of Hell — arena nova
      cine = 1.6; banner('WAKE OF HELL');
      shockwave(hero.position.x, hero.position.z, 22, 0xff5a1e);
      burst(hero.position.x, 1.6, hero.position.z, 0xff7a2a, 3, 70);
      for (const e of enemies) if (!e.dead) {
        const d = e.obj.position.distanceTo(hero.position);
        if (d < 22) damageEnemy(e, (1500 - d * 30) * (0.9 + Math.random() * 0.2), 'crit', 2.4);
      }
      shake = 1.2; hitStop = 0.14;
    }
    emit('ability', key);
  }
  function addStatus(short, color, time) {
    const found = P.statuses.find(s => s.short.split(' ')[0] === short.split(' ')[0]);
    if (found) { found.t = time; found.short = short; return; }
    P.statuses.push({ short, color, t: time, base: short.split(' ')[0] });
  }

  const angDiff = (a, b) => Math.atan2(Math.sin(a - b), Math.cos(a - b));
  function moveDir() {
    const d = new THREE.Vector3();
    if (keys.has('w') || keys.has('arrowup')) d.z -= 1;
    if (keys.has('s') || keys.has('arrowdown')) d.z += 1;
    if (keys.has('a') || keys.has('arrowleft')) d.x -= 1;
    if (keys.has('d') || keys.has('arrowright')) d.x += 1;
    return d.normalize();
  }
  function collide(pos, radius) {
    for (const c of colliders) {
      const dx = pos.x - c.x, dz = pos.z - c.z, d = Math.hypot(dx, dz), min = c.r + radius;
      if (d < min && d > 0.0001) { pos.x = c.x + (dx / d) * min; pos.z = c.z + (dz / d) * min; }
    }
    pos.x = Math.max(-ARENA, Math.min(ARENA, pos.x));
    pos.z = Math.max(-27, Math.min(27, pos.z));
  }

  /* ---------- boss behaviour ---------- */
  function wardenAI(e, dt) {
    const hpPct = e.hp / e.hpMax;
    const wantPhase = hpPct > 0.66 ? 1 : hpPct > 0.33 ? 2 : 3;
    if (wantPhase !== e.phase) {
      e.phase = wantPhase;
      banner(wantPhase === 2 ? 'SOULREND AURA UNBOUND' : 'CORRUPTED FLAME');
      shockwave(e.obj.position.x, e.obj.position.z, 16, 0xff5a1e);
      shake = 0.8;
      if (wantPhase === 2) for (let i = 0; i < 3; i++) spawn('reaver', e.obj.position.x + (i - 1) * 5, e.obj.position.z + 5);
      if (wantPhase === 3) for (let i = 0; i < 2; i++) spawn('wraith', e.obj.position.x + (i ? 6 : -6), e.obj.position.z + 4);
    }
    const aura = e.obj.userData.aura;
    aura.material.opacity = e.phase >= 2 ? 0.1 + Math.sin(performance.now() / 400) * 0.04 : 0;
    if (e.phase >= 2) { // Soulrend Aura — damage over time near the warden
      if (hero.position.distanceTo(e.obj.position) < 9 && Math.random() < dt * 1.6) damagePlayer(6);
    }
    const d = tmp.copy(hero.position).sub(e.obj.position); d.y = 0;
    const dist = d.length();
    e.obj.rotation.y = Math.atan2(d.x, d.z);
    if (e.casting > 0) { e.casting -= dt; return; }
    if (dist > e.def.range) {
      const step = e.def.speed * (e.phase === 3 ? 1.35 : 1) * dt;
      e.obj.position.add(d.normalize().multiplyScalar(step));
      collide(e.obj.position, 2.4);
    }
    if (e.cd > 0) { e.cd -= dt; return; }
    const pick = Math.random();
    if (dist < 8 && pick < 0.45) { // sweeping axe
      e.casting = 0.85; e.cd = e.phase === 3 ? 1.5 : 2.3;
      const ex = e.obj.position.x, ez = e.obj.position.z, face = e.obj.rotation.y;
      telegraph(ex + Math.sin(face) * 4, ez + Math.cos(face) * 4, 6.5, 0.8, () => {
        shockwave(ex + Math.sin(face) * 4, ez + Math.cos(face) * 4, 7);
        if (hero.position.distanceTo(new THREE.Vector3(ex + Math.sin(face) * 4, 0, ez + Math.cos(face) * 4)) < 7) damagePlayer(24);
        shake = Math.max(shake, 0.5);
      });
    } else if (pick < 0.75) { // ground slam under the player
      e.casting = 1.1; e.cd = e.phase === 3 ? 1.8 : 2.8;
      const px = hero.position.x, pz = hero.position.z;
      telegraph(px, pz, 5.5, 1.05, () => {
        burst(px, 1, pz, 0xff7a2a, 2, 40); shockwave(px, pz, 7);
        if (hero.position.distanceTo(new THREE.Vector3(px, 0, pz)) < 6) damagePlayer(32);
        shake = Math.max(shake, 0.85);
      });
    } else { // area denial — cinder pillars
      e.casting = 1.2; e.cd = 3.4;
      for (let i = 0; i < (e.phase === 3 ? 5 : 3); i++) {
        const a = Math.random() * Math.PI * 2, r = 5 + Math.random() * 14;
        const px = e.obj.position.x + Math.cos(a) * r, pz = e.obj.position.z + Math.sin(a) * r;
        telegraph(px, pz, 3.2, 1.3 + i * 0.16, () => {
          burst(px, 1, pz, 0xff5a1e, 1.4, 24);
          if (hero.position.distanceTo(new THREE.Vector3(px, 0, pz)) < 3.6) damagePlayer(18);
        });
      }
    }
  }

  /* ---------- enemy AI ---------- */
  function enemyAI(e, dt) {
    if (e.dead) { e.obj.position.y -= dt * 1.2; e.obj.rotation.z += dt * 1.1; return; }
    if (e.role === 'warden') return wardenAI(e, dt);
    e.anim += dt;
    if (e.stagger > 0) { e.stagger -= dt; e.obj.position.y = Math.sin(e.stagger * 40) * 0.05; return; }
    const d = tmp.copy(hero.position).sub(e.obj.position); d.y = 0;
    const dist = d.length();
    e.obj.rotation.y = Math.atan2(d.x, d.z);
    // ring positioning: hold a slot on a circle so enemies surround instead of stacking
    const slot = e.slot + (e.role === 'wraith' ? e.anim * 0.5 : 0);
    const want = new THREE.Vector3(hero.position.x + Math.sin(slot) * e.def.ring, 0, hero.position.z + Math.cos(slot) * e.def.ring);
    const toWant = want.sub(e.obj.position); toWant.y = 0;
    if (toWant.length() > 0.6) {
      e.obj.position.add(toWant.normalize().multiplyScalar(e.def.speed * dt));
      collide(e.obj.position, 0.9);
    }
    if (e.role === 'wraith') e.obj.position.y = 0.35 + Math.sin(e.anim * 3 + e.obj.userData.float) * 0.18;
    if (e.cd > 0) { e.cd -= dt; return; }
    if (dist <= e.def.range + 0.6) {
      e.cd = e.def.atkCd;
      if (e.role === 'acolyte') { // ranged bolt
        const o = e.obj.userData.orb;
        burst(e.obj.position.x, 3, e.obj.position.z, 0xff7a2a, 0.6, 10);
        const px = hero.position.x, pz = hero.position.z;
        telegraph(px, pz, 2.6, 0.85, () => {
          burst(px, 0.8, pz, 0xff7a2a, 1, 16);
          if (hero.position.distanceTo(new THREE.Vector3(px, 0, pz)) < 3) damagePlayer(e.def.dmg);
        });
        o.scale.setScalar(1.6);
      } else {
        e.casting = 0.3;
        setTimeout(() => {
          if (e.dead || P.dead) return;
          if (hero.position.distanceTo(e.obj.position) < e.def.range + 1.2) damagePlayer(e.def.dmg);
          burst(e.obj.position.x + (hero.position.x - e.obj.position.x) * 0.4, 1.5,
            e.obj.position.z + (hero.position.z - e.obj.position.z) * 0.4, 0xff8a3c, 0.6, 10);
        }, 300);
      }
    }
  }

  /* ---------- loop ---------- */
  const clock = new THREE.Clock();
  let raf = 0, disposed = false, hudT = 0;
  const camTarget = new THREE.Vector3(0, 0, 14);

  function resize() {
    const w = container.clientWidth || window.innerWidth, h = container.clientHeight || window.innerHeight;
    renderer.setSize(w, h, false);
    camera.aspect = w / h; camera.updateProjectionMatrix();
    if (composer) composer.setSize(w, h);
  }
  window.addEventListener('resize', resize);

  try { // bloom: the ember accents carry the composition
    const { EffectComposer } = await import(url('/examples/jsm/postprocessing/EffectComposer.js'));
    const { RenderPass } = await import(url('/examples/jsm/postprocessing/RenderPass.js'));
    const { UnrealBloomPass } = await import(url('/examples/jsm/postprocessing/UnrealBloomPass.js'));
    composer = new EffectComposer(renderer);
    composer.addPass(new RenderPass(scene, camera));
    bloom = new UnrealBloomPass(new THREE.Vector2(1, 1), 0.62, 0.7, 0.62);
    composer.addPass(bloom);
    RenderPassC = true;
  } catch (err) { composer = null; }
  resize();
  setStage(0);

  function tick() {
    if (disposed) return;
    raf = requestAnimationFrame(tick);
    let dt = Math.min(clock.getDelta(), 0.05);
    const now = performance.now() / 1000;
    if (hitStop > 0) { hitStop -= dt; dt *= 0.12; }

    /* player */
    if (!P.dead && cine <= 0) {
      const dir = moveDir();
      const accel = dir.lengthSq() > 0 ? 62 : 40;
      const target = dir.multiplyScalar(P.speed);
      P.vel.x += (target.x - P.vel.x) * Math.min(1, accel * dt / P.speed);
      P.vel.z += (target.z - P.vel.z) * Math.min(1, accel * dt / P.speed);
      if (P.vel.lengthSq() > 0.02) {
        hero.position.x += P.vel.x * dt; hero.position.z += P.vel.z * dt;
        collide(hero.position, 0.62);
        P.facing += angDiff(Math.atan2(P.vel.x, P.vel.z), P.facing) * Math.min(1, 14 * dt);
        P.walk += dt * Math.min(1, P.vel.length() / P.speed) * 10;
      } else P.walk += dt * 1.5;
      hero.rotation.y = P.facing;
    }
    hero.userData.legs.forEach((l, i) => { l.position.z = Math.sin(P.walk + i * Math.PI) * 0.26; l.position.y = 0.62 + Math.abs(Math.cos(P.walk + i * Math.PI)) * 0.06; });
    hero.userData.armL.rotation.x = Math.sin(P.walk + Math.PI) * 0.4;
    if (P.swing > 0) {
      P.swing -= dt;
      const k = Math.max(0, P.swing) * 4;
      hero.userData.armR.rotation.x = -1.9 + (1 - k) * 2.6;
      hero.userData.armR.rotation.z = -0.5 + (1 - k) * 1.1;
      hero.userData.trail.rotation.z = k * 2.4;
    } else {
      hero.userData.armR.rotation.x += (Math.sin(P.walk) * 0.35 - hero.userData.armR.rotation.x) * Math.min(1, 8 * dt);
      hero.userData.armR.rotation.z += (0 - hero.userData.armR.rotation.z) * Math.min(1, 8 * dt);
    }
    hero.userData.trail.material.opacity *= Math.pow(0.02, dt);
    hero.userData.cloak.rotation.x = -P.vel.length() * 0.012;
    P.iframe = Math.max(0, P.iframe - dt); P.dodgeCd = Math.max(0, P.dodgeCd - dt);
    P.bulwark = Math.max(0, P.bulwark - dt);
    for (const k in cds) cds[k] = Math.max(0, cds[k] - dt);
    P.wrath = Math.min(P.wrathMax, P.wrath + dt * 4.5);
    if (P.hp < P.hpMax && P.hp > 0) P.hp = Math.min(P.hpMax, P.hp + dt * 1.6);
    P.statuses = P.statuses.filter(s => (s.t -= dt) > 0);
    hero.userData.rim.intensity = 2.2 + Math.sin(now * 3) * 0.3 + (P.iframe > 0 ? 3 : 0);
    hero.visible = P.iframe > 0 ? Math.sin(now * 60) > -0.4 : true;

    /* enemies */
    for (const e of enemies) enemyAI(e, dt);
    for (let i = enemies.length - 1; i >= 0; i--) if (enemies[i].dead && enemies[i].obj.position.y < -6) enemies.splice(i, 1);

    /* stage flow */
    stageTimer += dt;
    const alive = enemies.filter(e => !e.dead).length;
    const s = stage();
    if (s === 'enter' && hero.position.z < 12) setStage(1);
    else if (s === 'wave1' && alive === 0 && stageTimer > 1) setStage(2);
    else if (s === 'advance' && hero.position.z < -4) setStage(3);
    else if (s === 'wave2' && alive === 0 && stageTimer > 1) setStage(4);
    else if (s === 'intro' && stageTimer > 3.4) setStage(5);
    else if (s === 'boss' && warden && warden.dead && stageTimer > 2.2) setStage(6);
    else if (s === 'reward' && loot.every(l => l.taken) && loot.length && stageTimer > 1) setStage(7);

    /* loot pickup */
    for (let i = loot.length - 1; i >= 0; i--) {
      const l = loot[i];
      l.t += dt; l.item.rotation.y += dt * 1.6; l.item.position.y = 0.7 + Math.sin(l.t * 2) * 0.12;
      if (!l.taken && hero.position.distanceTo(l.obj.position) < 2.2) {
        l.taken = true; scene.remove(l.obj);
        emit('loot', { rarity: l.rarity, label: l.def.label, sub: l.def.sub });
        burst(l.obj.position.x, 1, l.obj.position.z, l.def.c, 0.8, 18);
      }
    }

    /* telegraphs */
    for (let i = telegraphs.length - 1; i >= 0; i--) {
      const t = telegraphs[i]; t.t += dt;
      const k = Math.min(1, t.t / t.time);
      t.fill.scale.setScalar(0.2 + k * 0.8);
      t.ring.material.opacity = 0.3 + k * 0.55;
      if (t.t >= t.time) { scene.remove(t.ring, t.fill); t.cb && t.cb(); telegraphs.splice(i, 1); }
    }
    /* bursts + shockwaves */
    for (let i = bursts.length - 1; i >= 0; i--) {
      const b = bursts[i]; b.t += dt;
      if (b.light) { b.light.intensity = 9 * (1 - b.t / b.life); if (b.t >= b.life) { scene.remove(b.light); bursts.splice(i, 1); } continue; }
      const pos = b.p.geometry.attributes.position;
      for (let j = 0; j < b.vel.length; j++) {
        const v = b.vel[j];
        pos.array[j * 3] += v.x * dt; pos.array[j * 3 + 1] += v.y * dt; pos.array[j * 3 + 2] += v.z * dt;
        v.y -= 14 * dt; v.multiplyScalar(1 - 2.2 * dt);
      }
      pos.needsUpdate = true;
      b.p.material.opacity = Math.max(0, 1 - b.t / b.life);
      if (b.t >= b.life) { scene.remove(b.p); b.p.geometry.dispose(); bursts.splice(i, 1); }
    }
    for (let i = shockwaves.length - 1; i >= 0; i--) {
      const w = shockwaves[i]; w.t += dt;
      const k = w.t / w.life;
      w.m.scale.setScalar(1 + k * w.rMax);
      w.m.material.opacity = 0.55 * (1 - k);
      if (k >= 1) { scene.remove(w.m); shockwaves.splice(i, 1); }
    }

    /* ash + fire flicker */
    const ap = ash.geometry.attributes.position;
    for (let i = 0; i < ashN; i++) {
      ap.array[i * 3 + 1] += ashVel[i] * dt;
      ap.array[i * 3] += Math.sin(now * 0.4 + i) * dt * 0.3;
      if (ap.array[i * 3 + 1] > 26) { ap.array[i * 3 + 1] = 0; ap.array[i * 3] = hero.position.x + (Math.random() - 0.5) * 70; ap.array[i * 3 + 2] = hero.position.z + (Math.random() - 0.5) * 70; }
    }
    ap.needsUpdate = true;
    for (const b of braziers) {
      const f = 0.78 + Math.sin(now * 7 + b.seed) * 0.12 + Math.sin(now * 19 + b.seed * 3) * 0.08;
      b.light.intensity = b.base * f;
      b.flame.scale.setScalar(0.85 + f * 0.3);
      b.flame.rotation.y += dt * 2;
    }
    fillWarm.position.set(hero.position.x, 3, hero.position.z);
    fillWarm.intensity = 1.1 + (P.swing > 0 ? 6 : 0);

    /* camera — approved framing: elevated 3/4, hero low-of-center */
    const bossActive = warden && !warden.dead && stage() === 'boss';
    const zoomWant = cine > 0 ? 0.82 : bossActive ? 1.12 : 1;
    camZoom += (zoomWant - camZoom) * Math.min(1, 3 * dt);
    const focus = tmp.copy(hero.position);
    if (bossActive) focus.lerp(warden.obj.position, 0.28);
    camTarget.lerp(focus, Math.min(1, 6 * dt));
    shake = Math.max(0, shake - dt * 2.4);
    const sx = (Math.random() - 0.5) * shake * 1.6, sy = (Math.random() - 0.5) * shake * 1.2;
    camera.position.set(camTarget.x + CAM_OFF.x * camZoom + sx, CAM_OFF.y * camZoom + sy, camTarget.z + CAM_OFF.z * camZoom);
    camera.lookAt(camTarget.x, 2.4, camTarget.z - 4.6);

    /* floating numbers */
    for (let i = floaters.length - 1; i >= 0; i--) {
      const f = floaters[i]; f.t += dt;
      const p = f.pos.clone(); p.y += f.t * 1.6;
      p.project(camera);
      const w = container.clientWidth, h = container.clientHeight;
      f.el.style.left = (p.x * 0.5 + 0.5) * w + 'px';
      f.el.style.top = (-p.y * 0.5 + 0.5) * h + 'px';
      f.el.style.opacity = String(Math.max(0, 1 - f.t / f.life));
      if (f.t >= f.life) { f.el.remove(); floaters.splice(i, 1); }
    }

    /* HUD push */
    hudT += dt;
    if (hudT > 0.05 && hooks.onHud) {
      hudT = 0;
      const near = enemies.filter(e => !e.dead).slice(0, 14).map(e => ({
        x: (e.obj.position.x - hero.position.x) / 34, y: (e.obj.position.z - hero.position.z) / 34,
        elite: !!e.def.elite, kind: e.role
      }));
      hooks.onHud({
        hp: P.hp / P.hpMax * 100, wrath: P.wrath / P.wrathMax * 100, xp: P.xp, level: P.level,
        cds: { LMB: cds.LMB / CD.LMB * 100, Q: cds.Q / CD.Q * 100, W: cds.W / CD.W * 100, E: cds.E / CD.E * 100, R: cds.R / CD.R * 100 },
        statuses: P.statuses.map(st => ({ short: st.short.split(' ')[0] + ' ' + Math.ceil(st.t) + 's', color: st.color })),
        boss: warden ? { alive: !warden.dead, hp: Math.max(0, warden.hp / warden.hpMax * 100), phase: warden.phase } : null,
        objective: { text: STAGES[stageIdx].text, index: stageIdx, total: STAGES.length - 1, alive },
        stage: stage(), banner: bannerT > 0 ? bannerText : '', cine: cine > 0 ? 1 : 0,
        dead: P.dead, blips: near, loot: loot.filter(l => !l.taken).map(l => ({
          x: (l.obj.position.x - hero.position.x) / 34, y: (l.obj.position.z - hero.position.z) / 34, rarity: l.rarity }))
      });
    }
    bannerT = Math.max(0, bannerT - dt);
    cine = Math.max(0, cine - dt);

    if (composer) composer.render(); else renderer.render(scene, camera);
  }
  tick();

  return {
    dispose() {
      disposed = true; cancelAnimationFrame(raf);
      window.removeEventListener('keydown', onDown); window.removeEventListener('keyup', onUp);
      window.removeEventListener('resize', resize);
      renderer.domElement.removeEventListener('pointerdown', onPointer);
      renderer.domElement.removeEventListener('contextmenu', onCtx);
      renderer.dispose(); container.innerHTML = '';
    },
    resize
  };
}
