(function(){const t=document.createElement("link").relList;if(t&&t.supports&&t.supports("modulepreload"))return;for(const c of document.querySelectorAll('link[rel="modulepreload"]'))a(c);new MutationObserver(c=>{for(const n of c)if(n.type==="childList")for(const r of n.addedNodes)r.tagName==="LINK"&&r.rel==="modulepreload"&&a(r)}).observe(document,{childList:!0,subtree:!0});function i(c){const n={};return c.integrity&&(n.integrity=c.integrity),c.referrerPolicy&&(n.referrerPolicy=c.referrerPolicy),c.crossOrigin==="use-credentials"?n.credentials="include":c.crossOrigin==="anonymous"?n.credentials="omit":n.credentials="same-origin",n}function a(c){if(c.ep)return;c.ep=!0;const n=i(c);fetch(c.href,n)}})();const p="http://localhost:8080",e={activeTab:"overview",connectionStatus:"unknown",switches:[],metrics:{packetsProcessed:0,activeFlows:0,uptime:"0s"},logs:[]};async function o(s){try{const t=await fetch(`${p}${s}`);if(!t.ok)throw new Error(`HTTP ${t.status}`);return await t.json()}catch(t){return console.error(`API GET ${s} failed:`,t),null}}function d(){const s=document.querySelector("#app");s.innerHTML=`
    <header class="dashboard-header">
      <h1>NetOrc SDN Controller Dashboard</h1>
      <span id="connection-badge" class="badge badge--${e.connectionStatus}">${e.connectionStatus}</span>
    </header>
    <nav class="tabs">
      <button class="tab ${e.activeTab==="overview"?"tab--active":""}" data-tab="overview">Overview</button>
      <button class="tab ${e.activeTab==="switches"?"tab--active":""}" data-tab="switches">Switches</button>
      <button class="tab ${e.activeTab==="metrics"?"tab--active":""}" data-tab="metrics">Metrics</button>
      <button class="tab ${e.activeTab==="logs"?"tab--active":""}" data-tab="logs">Logs</button>
    </nav>
    <main class="tab-content" id="tab-content"></main>
  `,document.querySelectorAll(".tab").forEach(t=>t.addEventListener("click",()=>{e.activeTab=t.dataset.tab,d(),h(),l()}))}function u(){const s=document.getElementById("tab-content");s.innerHTML=`
    <section class="overview-grid">
      <div class="card"><h3>Packets Processed</h3><p class="card-value">${e.metrics.packetsProcessed.toLocaleString()}</p></div>
      <div class="card"><h3>Active Flows</h3><p class="card-value">${e.metrics.activeFlows}</p></div>
      <div class="card"><h3>Switches</h3><p class="card-value">${e.switches.length}</p></div>
      <div class="card"><h3>Uptime</h3><p class="card-value">${e.metrics.uptime}</p></div>
    </section>
    <section class="section">
      <h2>Connected Switches</h2>
      <table class="data-table">
        <thead><tr><th>DPID</th><th>Status</th><th>Ports</th><th>Flows</th></tr></thead>
        <tbody>
          ${e.switches.length===0?'<tr><td colspan="4">No switches connected</td></tr>':e.switches.map(t=>`
              <tr><td>${t.dpid||t.id||"-"}</td><td><span class="badge badge--connected">connected</span></td><td>${t.ports||0}</td><td>${t.flows||0}</td></tr>
            `).join("")}
        </tbody>
      </table>
    </section>
  `}function v(){const s=document.getElementById("tab-content");s.innerHTML=`
    <section class="section">
      <h2>Network Topology</h2>
      <div class="topology">
        ${e.switches.length===0?'<p class="empty-state">No switches discovered</p>':e.switches.map(t=>`
            <div class="switch-node">
              <strong>${t.dpid||t.id||"unknown"}</strong>
              <span>Ports: ${t.ports||0}</span>
              <span>Flows: ${t.flows||0}</span>
            </div>
          `).join("")}
      </div>
    </section>
  `}function m(){const s=document.getElementById("tab-content");s.innerHTML=`
    <section class="section">
      <h2>Real-Time Metrics</h2>
      <div class="metrics-grid">
        <div class="metric-item"><label>Packets Processed</label><span>${e.metrics.packetsProcessed.toLocaleString()}</span></div>
        <div class="metric-item"><label>Active Flows</label><span>${e.metrics.activeFlows}</span></div>
        <div class="metric-item"><label>Control Channel Uptime</label><span>${e.metrics.uptime}</span></div>
        <div class="metric-item"><label>Connected Switches</label><span>${e.switches.length}</span></div>
      </div>
    </section>
    <section class="section">
      <h2>Switch Details</h2>
      <pre class="raw-json">${JSON.stringify(e.switches,null,2)}</pre>
    </section>
  `}function b(){const s=document.getElementById("tab-content");s.innerHTML=`
    <section class="section">
      <h2>Controller Logs</h2>
      <div class="log-container">
        ${e.logs.length===0?'<p class="empty-state">No log entries yet</p>':e.logs.map(t=>`<div class="log-entry log-entry--${t.level||"info"}"><span class="log-time">${t.timestamp||""}</span><span class="log-msg">${t.message||t}</span></div>`).join("")}
      </div>
    </section>
  `}function h(){switch(e.activeTab){case"overview":return u();case"switches":return v();case"metrics":return m();case"logs":return b()}}async function l(){const s=await o("/health");e.connectionStatus=s?"connected":"disconnected";const t=await o("/api/switches");t&&(e.switches=Array.isArray(t)?t:[]);const i=await o("/api/metrics");i&&(e.metrics={...e.metrics,...i});const a=await o("/api/logs");a&&(e.logs=Array.isArray(a)?a:[]),document.getElementById("connection-badge").className=`badge badge--${e.connectionStatus}`,document.getElementById("connection-badge").textContent=e.connectionStatus}d();h();l();setInterval(l,5e3);
