const USERS = {
  "operador@sentry.demo": { role:"operador", name:"Carlos Mendes", gestor:"Juliana Prado" },
  "gestor@sentry.demo":   { role:"gestor",   name:"Juliana Prado" }
};
const state = {
  user:null,
  belts:[
    { id:1, nome:"Esteira 1", setor:"Embalagem", status:"run",  count:1284, equip:[{n:"Sensor VL53L0X",s:"ok"},{n:"Relé 5V",s:"ok"},{n:"Motor",s:"ok"}] },
    { id:2, nome:"Esteira 2", setor:"Separação", status:"warn", count:963,  equip:[{n:"Sensor VL53L0X",s:"warn",note:"Leitura instável"},{n:"Relé 5V",s:"ok"},{n:"Motor",s:"ok"}] },
    { id:3, nome:"Esteira 3", setor:"Expedição", status:"run",  count:1517, equip:[{n:"Sensor VL53L0X",s:"ok"},{n:"Relé 5V",s:"ok"},{n:"Motor",s:"ok"}] }
  ],
  feed0:0,
  tasks:[
    { id:"t1", txt:"Verificar esteiras no início do turno", done:false },
    { id:"t2", txt:"Conferir equipamentos e sensores", done:false },
    { id:"t3", txt:"Simular parada pelo sistema", done:false, action:"simular" }
  ],
  stops:[], feed:[
    { at:new Date(Date.now()-42*60e3), txt:"Esteira 2: leitura instável no sensor", lvl:"warn" },
    { at:new Date(Date.now()-95*60e3), txt:"Wi-Fi do ESP32 reconectado automaticamente", lvl:"info" },
    { at:new Date(Date.now()-180*60e3), txt:"Turno iniciado, motores ligados", lvl:"info" }
  ], period:"dia", stopTarget:null
};

(function seedStops(){
  const ops=["Carlos Mendes","Rafaela Souza","Diego Alves"], gest=["Juliana Prado","Marcos Teixeira"];
  const reasons=["Objeto na zona de risco","Falha de equipamento","Manutenção preventiva","Troca de produto"];
  const equips=["Sensor VL53L0X","Relé 5V","Motor"];
  const rnd=mulberry(42), now=Date.now();
  for(let i=0;i<46;i++){
    const hoursAgo = i<4 ? 1+rnd()*10 : rnd()*24*170;
    const sys = rnd()<.45;
    state.stops.push({
      at:new Date(now-hoursAgo*3600e3), belt:1+Math.floor(rnd()*3),
      equip: sys?"Sensor VL53L0X":equips[Math.floor(rnd()*3)],
      reason: sys?"Sensor detectou objeto na zona de segurança":reasons[Math.floor(rnd()*4)],
      origin: sys?"Sistema":"Manual", op:ops[Math.floor(rnd()*3)], gestor:gest[Math.floor(rnd()*2)],
      dur: Math.round(2+rnd()*28)
    });
  }
  state.stops.sort((a,b)=>b.at-a.at);
})();
function mulberry(a){return function(){a|=0;a=a+0x6D2B79F5|0;let t=Math.imul(a^a>>>15,1|a);t=t+Math.imul(t^t>>>7,61|t)^t;return((t^t>>>14)>>>0)/4294967296}}

function go(view){
  if((view==="operador"||view==="gestor") && (!state.user || state.user.role!==view)) view = state.user ? state.user.role : "login";
  document.querySelectorAll(".view").forEach(v=>v.classList.toggle("active", v.id===view));
  window.scrollTo(0,0);
  if(view==="operador") renderAndon();
  if(view==="gestor") renderBI();
}
document.addEventListener("click",e=>{
  const g=e.target.closest("[data-go]"); if(g){ e.preventDefault(); go(g.dataset.go); }
  if(e.target.closest("[data-logout]")){ state.user=null; go("vendas"); toast("Você saiu da plataforma"); }
  const p=e.target.closest("[data-plan]"); if(p) toast(`Plano ${p.dataset.plan} selecionado (demonstração)`);
  const d=e.target.closest("[data-demo]"); if(d){ email.value=d.dataset.demo+"@sentry.demo"; senha.value="1234"; loginErr.textContent=""; }
});

loginForm.addEventListener("submit",e=>{
  e.preventDefault();
  const u=USERS[email.value.trim().toLowerCase()];
  if(!email.value || !senha.value){ loginErr.textContent="Preencha e-mail e senha para entrar."; return; }
  if(!u || senha.value!=="1234"){ loginErr.textContent="E-mail ou senha incorretos. Confira os dados e tente de novo."; return; }
  state.user=u; loginErr.textContent=""; senha.value="";
  document.querySelectorAll("[data-username]").forEach(n=>n.textContent=u.name);
  go(u.role);
});

(function field(){
  const cv=document.getElementById("field"), hero=document.getElementById("hero"), frame=document.getElementById("frame");
  const ctx=cv.getContext("2d"), reduce=matchMedia("(prefers-reduced-motion: reduce)").matches;
  const COLORS=["#7031DB","#7031DB","#8D5CF6","#FF8AD4","#FF8AD4","#B9B4F0","#6EE7A6"];
  const D=Math.PI/180;
  const GEARS=[{cx:16,cy:23,n:10,ro:11.2,ri:8.6,h:3.4,ph:4.8*D,dir:1,sp:1},{cx:29,cy:13.2,n:8,ro:7.4,ri:5.4,h:2.2,ph:-7.3*D,dir:-1,sp:10/8}];
  function radius(g,a){ const t=2*Math.PI/g.n; const f=((((a-g.ph)%t)+t)%t)/t;
    if(f<.18) return g.ri+(g.ro-g.ri)*f/.18; if(f<=.5) return g.ro; if(f<.68) return g.ro-(g.ro-g.ri)*(f-.5)/.18; return g.ri; }
  let W=0,H=0,S=1,ox=0,oy=0,parts=[],mouse={x:-1e4,y:-1e4},running=true,t0=performance.now();
  function build(){
    const dpr=Math.min(2,devicePixelRatio||1);
    W=hero.clientWidth; H=hero.clientHeight; cv.width=W*dpr; cv.height=H*dpr; ctx.setTransform(dpr,0,0,dpr,0,0);
    const mobile=W<760;
    S= mobile ? Math.min(W*.8,H*.36)/40 : Math.min(W*.44,H*.82)/40;
    ox= mobile ? W/2-20*S : W*.74-20*S;
    oy= mobile ? H-40*S-60 : H/2-20*S;
    const step= mobile ? .66 : .48, old=parts; parts=[];
    GEARS.forEach(g=>{
      for(let x=g.cx-g.ro;x<=g.cx+g.ro;x+=step) for(let y=g.cy-g.ro;y<=g.cy+g.ro;y+=step){
        const dx=x-g.cx, dy=y-g.cy, r=Math.hypot(dx,dy), a=Math.atan2(dy,dx);
        if(r<g.h+.5 || r>radius(g,a)) continue;
        const jx=(Math.random()-.5)*step*.6, jy=(Math.random()-.5)*step*.6;
        const o=old[parts.length];
        parts.push({g, rr:r, a0:a, jx, jy, seed:Math.random()*6.28,
          c:COLORS[(Math.random()*COLORS.length)|0], s:1.2+Math.random()*1.6,
          x:o?o.x:Math.random()*W, y:o?o.y:Math.random()*H, vx:0, vy:0});
      }
    });
    for(let i=0;i<70;i++) parts.push({free:true,x:Math.random()*W,y:Math.random()*H,vx:(Math.random()-.5)*.25,vy:-.1-Math.random()*.25,c:COLORS[(Math.random()*COLORS.length)|0],s:1+Math.random()*1.4,seed:Math.random()*6.28});
    parts.sort((a,b)=>a.c<b.c?-1:1);
  }
  function target(p,t){
    const g=p.g, ang=p.a0+g.dir*g.sp*t*.22;
    return [ox+(g.cx+p.rr*Math.cos(ang)+p.jx)*S + Math.sin(t*1.1+p.seed)*1.4, oy+(g.cy+p.rr*Math.sin(ang)+p.jy)*S + Math.cos(t*.9+p.seed)*1.4];
  }
  function frameFx(t){
    ctx.clearRect(0,0,W,H); let last="";
    for(const p of parts){
      if(p.free){ p.x+=p.vx; p.y+=p.vy; if(p.y<-10){p.y=H+10;p.x=Math.random()*W;} }
      else{
        const [tx,ty]=target(p,t);
        p.vx+=(tx-p.x)*.012; p.vy+=(ty-p.y)*.012;
        const dx=p.x-mouse.x, dy=p.y-mouse.y, d=Math.hypot(dx,dy);
        if(d<120 && d>0){ const f=(1-d/120)*5.5; p.vx+=dx/d*f; p.vy+=dy/d*f; }
        p.vx*=.86; p.vy*=.86; p.x+=p.vx; p.y+=p.vy;
      }
      if(p.c!==last){ ctx.fillStyle=p.c; last=p.c; }
      ctx.fillRect(p.x,p.y,p.s,p.s);
    }
  }
  function loop(now){
    if(running && document.getElementById("vendas").classList.contains("active")) frameFx((now-t0)/1000);
    requestAnimationFrame(loop);
  }
  build();
  if(reduce){ parts.forEach(p=>{ if(!p.free){ [p.x,p.y]=target(p,0);} }); frameFx(0); }
  else requestAnimationFrame(loop);
  addEventListener("resize",()=>{ build(); if(reduce){ parts.forEach(p=>{ if(!p.free){ [p.x,p.y]=target(p,0);} }); frameFx(0);} });
  hero.addEventListener("pointermove",e=>{ const r=cv.getBoundingClientRect(); mouse.x=e.clientX-r.left; mouse.y=e.clientY-r.top; });
  hero.addEventListener("pointerleave",()=>{ mouse.x=mouse.y=-1e4; });
  new IntersectionObserver(es=>{ running=es[0].isIntersecting; }).observe(hero);

  if(!reduce){
    const tilt=()=>{ const r=frame.getBoundingClientRect(), vh=innerHeight;
      const k=Math.max(0,Math.min(1,1-(r.top-vh*.2)/(vh*.6)));
      frame.style.transform=`perspective(1400px) rotateX(${(1-k)*18}deg) scale(${.92+.08*k})`; };
    addEventListener("scroll",tilt,{passive:true}); tilt();
  }
})();

const STATUS={run:"Rodando",warn:"Atenção",stop:"Parada"};
function renderAndon(){
  andon.innerHTML=state.belts.map(b=>`
    <article class="belt-card s-${b.status}">
      <div class="status-bar"><span>${b.nome}</span><span class="pulse">${STATUS[b.status]}</span></div>
      <div class="belt-body">
        <div><div class="big">${b.count.toLocaleString("pt-BR")}</div><div class="sub">peças hoje, setor ${b.setor}</div></div>
        ${b.status==="stop"?`<div class="belt-note">${b.stopReason}</div>`:""}
        <ul class="equip">${b.equip.map(q=>`<li><span><i class="dot d-${q.s}"></i>${q.n}</span><span class="muted">${q.note||(q.s==="ok"?"Normal":q.s==="stop"?"Acionado":"Verificar")}</span></li>`).join("")}</ul>
        ${b.status==="stop"
          ? `<button class="btn btn-gold" onclick="resume(${b.id})">Retomar esteira</button>`
          : `<button class="btn btn-stop" onclick="openStop(${b.id})">Parar esteira</button>`}
      </div>
    </article>`).join("");
  tasks.innerHTML=state.tasks.map(t=>`
    <div class="task ${t.done?"done":""}">
      <input type="checkbox" id="${t.id}" ${t.done?"checked":""} onchange="toggleTask('${t.id}',this.checked)">
      <label for="${t.id}">${t.txt}</label>
      ${t.action==="simular"?`<button class="btn btn-line btn-sm" onclick="simulate()">Simular</button>`:""}
    </div>`).join("");
  feed.innerHTML = state.feed.length
    ? state.feed.map(f=>`<li><span class="tag t-${f.lvl}">${{err:"Erro",warn:"Aviso",info:"Info"}[f.lvl]}</span><span class="muted">${hhmm(f.at)}</span><b>${f.txt}</b></li>`).join("")
    : `<li class="muted">Nenhum evento neste turno. As paradas e retomadas aparecem aqui.</li>`;
}
function toggleTask(id,v){ state.tasks.find(t=>t.id===id).done=v; renderAndon(); }
function openStop(id){
  state.stopTarget=id; const b=state.belts.find(x=>x.id===id);
  stopSub.textContent=`${b.nome}, setor ${b.setor}. A esteira para imediatamente e a parada fica registrada.`;
  stopModal.classList.add("open"); stopReason.focus();
}
stopCancel.onclick=()=>stopModal.classList.remove("open");
stopModal.addEventListener("click",e=>{ if(e.target===stopModal) stopModal.classList.remove("open"); });
document.addEventListener("keydown",e=>{ if(e.key==="Escape") stopModal.classList.remove("open"); });
stopConfirm.onclick=()=>{
  stopBelt(state.stopTarget, stopReason.value, stopEquip.value, "Manual");
  stopModal.classList.remove("open");
};

const ESTEIRA_REAL = 1;
const ESP_URL = (()=>{ const ip=new URLSearchParams(location.search).get("esp"); return ip ? "http://"+ip : ""; })();
let espConectado = false;

function esp(rota){
  return fetch(ESP_URL+rota,{method:"POST"})
    .then(r=>{ if(!r.ok) throw new Error(r.status); return r.json(); })
    .catch(()=>{ toast("Sem conexão com a esteira"); });
}
function mostrarConexao(ok, s){
  espConectado = ok;
  const el=document.getElementById("conexao");
  if(!el) return;
  el.textContent = ok
    ? `ESP32 conectado (IP ${s.ip}, sinal ${s.rssi} dBm). A Esteira 1 é a esteira real; as Esteiras 2 e 3 usam valores ilustrativos.`
    : "Sem conexão com o ESP32. Tentando novamente a cada segundo…";
  el.style.color = ok ? "" : "var(--stop)";
}

function stopBelt(id, reason, equip, origin, semComando){
  const b=state.belts.find(x=>x.id===id);
  if(id===ESTEIRA_REAL && !semComando) esp("/parar");
  b.status="stop"; b.stopReason=reason; b.stoppedAt=Date.now();
  b.equip.forEach(q=>{ if(q.n==="Relé 5V") q.s="stop"; });
  const u=state.user||{name:"Sistema",gestor:"Juliana Prado"};
  state.stops.unshift({ at:new Date(), belt:id, equip, reason, origin, op:u.name, gestor:u.gestor||"Juliana Prado", dur:0, live:true });
  state.feed.unshift({ at:new Date(), txt:`${b.nome} parada (${origin.toLowerCase()}): ${reason}`, lvl:"err" });
  renderAndon(); toast(`${b.nome} parada`);
}
function resume(id, semComando){
  const b=state.belts.find(x=>x.id===id);
  if(id===ESTEIRA_REAL && !semComando) esp("/ligar");
  const s=state.stops.find(x=>x.belt===id && x.live);
  if(s){ s.dur=Math.max(1,Math.round((Date.now()-b.stoppedAt)/60000)); s.live=false; }
  b.status="run"; b.stopReason="";
  b.equip.forEach(q=>{ if(q.s==="stop") q.s="ok"; if(q.s==="warn"){ q.s="ok"; q.note=""; } });
  state.feed.unshift({ at:new Date(), txt:`${b.nome} retomada`, lvl:"info" });
  renderAndon(); toast(`${b.nome} retomada`);
}
function simulate(){
  const b=state.belts.find(x=>x.status!=="stop");
  if(!b){ toast("Todas as esteiras já estão paradas"); return; }
  b.equip.find(q=>q.n==="Sensor VL53L0X").s="stop";
  stopBelt(b.id,"Sensor detectou objeto na zona de segurança","Sensor VL53L0X","Sistema");
  state.tasks.find(t=>t.action==="simular").done=true; renderAndon();
}

setInterval(()=>{
  state.belts.forEach(b=>{ if(b.status!=="stop" && b.id!==ESTEIRA_REAL) b.count+=Math.floor(Math.random()*3); });
  atualizarContagens();
},2000);
function atualizarContagens(){
  if(document.getElementById("operador").classList.contains("active")){
    document.querySelectorAll("#andon .big").forEach((el,i)=>el.textContent=state.belts[i].count.toLocaleString("pt-BR"));
  }
}

let ultimasParadas=null;
function lerEsteira(){
  fetch(ESP_URL+"/status").then(r=>r.json()).then(s=>{
    mostrarConexao(true, s);
    const b=state.belts.find(x=>x.id===ESTEIRA_REAL);
    const sensor=b.equip.find(q=>q.n==="Sensor VL53L0X");
    const primeira = ultimasParadas===null;
    b.count=s.contagem;
    sensor.s = s.sensor_ok ? (sensor.s==="warn" ? "ok" : sensor.s) : "warn";
    sensor.note = s.sensor_ok ? "" : "Sensor não encontrado";

    if(s.estado==="parada_seguranca" && b.status!=="stop"){
      sensor.s="stop";
      if(primeira || s.paradas_seguranca<=ultimasParadas){ b.status="stop"; b.stopReason="Sensor detectou objeto na zona de segurança"; b.stoppedAt=Date.now(); b.equip.forEach(q=>{ if(q.n==="Relé 5V") q.s="stop"; }); renderAndon(); }
      else stopBelt(ESTEIRA_REAL,"Sensor detectou objeto na zona de segurança","Sensor VL53L0X","Sistema",true);
    } else if(s.estado==="parado" && b.status!=="stop"){
      b.status="stop"; b.stopReason="Parada manual"; b.stoppedAt=Date.now();
      b.equip.forEach(q=>{ if(q.n==="Relé 5V") q.s="stop"; });
      renderAndon();
    } else if(s.estado==="operando" && b.status==="stop"){
      resume(ESTEIRA_REAL,true);
    } else {
      atualizarContagens();
    }
    ultimasParadas=s.paradas_seguranca;
  }).catch(()=>{ if(espConectado || ultimasParadas===null) mostrarConexao(false); });
}
setInterval(lerEsteira,1000); lerEsteira();

state.belts.forEach(b=>fBelt.insertAdjacentHTML("beforeend",`<option value="${b.id}">${b.nome}</option>`));
periodSeg.addEventListener("click",e=>{
  const b=e.target.closest("button"); if(!b) return;
  state.period=b.dataset.p;
  periodSeg.querySelectorAll("button").forEach(x=>x.setAttribute("aria-pressed",x===b));
  renderBI();
});
fBelt.onchange=renderBI; fEquip.onchange=renderBI;

const PERIODS={
  hora:{n:12,ms:3600e3,  title:"Produção por hora",  lab:d=>d.getHours()+"h", per:60},
  dia:{n:7,ms:864e5,     title:"Produção por dia",   lab:d=>["dom","seg","ter","qua","qui","sex","sáb"][d.getDay()], per:950},
  semana:{n:8,ms:6048e5, title:"Produção por semana",lab:d=>`${d.getDate()}/${d.getMonth()+1}`, per:6400},
  mes:{n:6,ms:2592e6,    title:"Produção por mês",   lab:d=>["jan","fev","mar","abr","mai","jun","jul","ago","set","out","nov","dez"][d.getMonth()], per:27000}
};
function renderBI(){
  const P=PERIODS[state.period], belt=fBelt.value, eq=fEquip.value, now=Date.now();
  const beltIds = belt==="all" ? [1,2,3] : [+belt];
  const labels=[], values=[];
  for(let i=P.n-1;i>=0;i--){
    const d=new Date(now-i*P.ms); labels.push(P.lab(d));
    let v=0; beltIds.forEach(id=>{ const r=mulberry(id*997+Math.floor((now-i*P.ms)/P.ms))(); v+=Math.round(P.per*(.75+r*.5)); });
    values.push(v);
  }
  const from=now-P.n*P.ms;
  const stops=state.stops.filter(s=>s.at>=from && beltIds.includes(s.belt) && (eq==="all"||s.equip===eq));
  const prod=values.reduce((a,b)=>a+b,0);
  const down=stops.reduce((a,s)=>a+s.dur,0);
  const totalMin=P.n*P.ms/60000*beltIds.length;
  kProd.textContent=prod.toLocaleString("pt-BR");
  kStops.textContent=stops.length;
  kDown.textContent= down>=60 ? `${Math.floor(down/60)}h ${down%60}min` : `${down} min`;
  kAvail.textContent=(Math.max(0,100-down/totalMin*100)).toFixed(1).replace(".",",")+"%";
  prodTitle.textContent=P.title;
  chartProd.innerHTML=bars(labels,values);
  const per=[1,2,3].filter(id=>beltIds.includes(id));
  chartStops.innerHTML=bars(per.map(id=>"Esteira "+id), per.map(id=>stops.filter(s=>s.belt===id).length), true);
  stopTable.innerHTML = stops.length ? stops.slice(0,25).map(s=>`
    <tr><td>${fmt(s.at)}</td><td>Esteira ${s.belt}</td><td>${s.equip}</td><td>${s.reason}</td>
    <td><span class="tag ${s.origin==="Sistema"?"t-sys":"t-man"}" style="margin:0">${s.origin}</span></td>
    <td>${s.op}</td><td>${s.gestor}</td><td>${s.live?"Em andamento":s.dur+" min"}</td></tr>`).join("")
    : `<tr><td colspan="8" class="empty">Nenhuma parada neste período. Ajuste o período ou os filtros para ver o histórico.</td></tr>`;
}
function bars(labels,values,stopColor){
  const W=600,H=240,pad=30,bw=(W-pad)/labels.length,max=Math.max(1,...values);
  const color=stopColor?"var(--stop)":"var(--gold)";
  return `<svg viewBox="0 0 ${W} ${H}" role="img" aria-label="Gráfico de barras">
    <line x1="0" y1="${H-pad}" x2="${W}" y2="${H-pad}" stroke="var(--line)"/>
    ${values.map((v,i)=>{ const h=(H-pad-28)*v/max, x=pad/2+i*bw+bw*.18, w=bw*.64, y=H-pad-h;
      return `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="3" fill="${color}" opacity="${i===values.length-1||stopColor?1:.6}"/>
      <text x="${x+w/2}" y="${y-8}" text-anchor="middle" font-size="13" fill="var(--ink)" font-family="Manrope,sans-serif">${v.toLocaleString("pt-BR")}</text>
      <text x="${x+w/2}" y="${H-10}" text-anchor="middle" font-size="13" fill="var(--ink-soft)" font-family="Manrope,sans-serif">${labels[i]}</text>`; }).join("")}
  </svg>`;
}

const ICON={sun:'<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4"/></svg>',
            moon:'<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M21 12.8A9 9 0 1 1 11.2 3a7 7 0 0 0 9.8 9.8z"/></svg>'};
function applyZone(z){
  document.querySelectorAll("#operador,#gestor,#stopModal").forEach(el=>el.dataset.zone=z);
  document.querySelectorAll("[data-theme-toggle]").forEach(b=>b.innerHTML= z==="dark" ? ICON.sun+"<span class=\"tl\">Tema claro</span>" : ICON.moon+"<span class=\"tl\">Tema escuro</span>");
  try{ localStorage.setItem("sentry-zone",z); }catch(e){}
  if(document.getElementById("gestor").classList.contains("active")) renderBI();
}
let savedZone="dark"; try{ savedZone=localStorage.getItem("sentry-zone")||"dark"; }catch(e){}
applyZone(savedZone==="light"?"light":"dark");
document.addEventListener("click",e=>{ if(e.target.closest("[data-theme-toggle]")) applyZone(document.getElementById("operador").dataset.zone==="dark"?"light":"dark"); });

function hhmm(d){ return d.toLocaleTimeString("pt-BR",{hour:"2-digit",minute:"2-digit"}); }
function fmt(d){ return d.toLocaleDateString("pt-BR",{day:"2-digit",month:"2-digit"})+" "+hhmm(d); }
let tt; function toast(msg){ toastEl.textContent=msg; toastEl.classList.add("show"); clearTimeout(tt); tt=setTimeout(()=>toastEl.classList.remove("show"),2400); }
const toastEl=document.getElementById("toast");
function clock(){ const t=new Date().toLocaleString("pt-BR",{weekday:"long",day:"2-digit",month:"long",hour:"2-digit",minute:"2-digit"}); document.querySelectorAll("[data-clock]").forEach(c=>c.textContent=t); }
clock(); setInterval(clock,30000);
