const state = { modules: {} };
let chart = null;
let selectedSensor = null;
let navigation = {
  level: "module",
  moduleKey: null,
  typeKey: null,
  module: null,
  type: null,
};

const MODULE_NAMES = {
  0: "Bond Head",
  1: "Stage",
  2: "Heater",
  3: "Vacuum",
  4: "Motor/Drive",
  5: "Vision",
  6: "Environment",
  7: "Power",
};
const TYPE_NAMES = {
  0: "Temperature",
  2: "Force",
  3: "Ultrasonic Power",
  4: "Position Encoder",
  5: "Motor Current",
  8: "Vibration",
  9: "Vacuum Pressure",
  15: "Power Consumption",
};

function main() {
  initChart();
  setupBackButton();
  renderList(); // 💡 초기 리스트 렌더링 추가
  connectWebSocket();
}

function initChart() {
  const canvas = document.getElementById("chartCanvas");
  if (!canvas) return;
  const ctx = canvas.getContext("2d");
  chart = new Chart(ctx, {
    type: "line",
    data: { datasets: [] },
    options: {
      animation: false,
      maintainAspectRatio: false,
      scales: {
        x: { display: false },
        y: {
          grid: { color: "rgba(255,255,255,0.05)" },
          ticks: { color: "#89919d" },
        },
      },
      plugins: { legend: { display: false } },
    },
  });
}

function connectWebSocket() {
  // 💡 127.0.0.1로 변경하여 접속 안정성 확보
  const ws = new WebSocket("ws://127.0.0.1:8080");

  ws.onopen = () => console.log("✅ 대시보드가 서버에 연결되었습니다.");
  ws.onerror = (err) =>
    console.error("❌ 서버 연결 실패. Relay 서버를 확인하세요.");

  ws.onmessage = (e) => {
    const json = JSON.parse(e.data);

    if (json.timestamp) {
      const latency = Date.now() - json.timestamp;
      const display = document.querySelector("#latencyDisplay span:last-child");
      if (display) display.textContent = latency + "ms";
    }

    json.sensors.forEach((s) => {
      if (!state.modules[s.m])
        state.modules[s.m] = { types: {}, anomaly: false };
      if (!state.modules[s.m].types[s.t])
        state.modules[s.m].types[s.t] = { sensors: {}, anomaly: false };
      if (!state.modules[s.m].types[s.t].sensors[s.id])
        state.modules[s.m].types[s.t].sensors[s.id] = {
          history: [],
          anomaly: false,
        };

      const sensor = state.modules[s.m].types[s.t].sensors[s.id];
      sensor.id = s.id;
      sensor.m = s.m;
      sensor.t = s.t;
      sensor.history.push({ x: Date.now(), y: s.avg, value: s.value });
      if (sensor.history.length > 50) sensor.history.shift();
      sensor.anomaly = s.anomaly === 1;

      // 상위 객체에 이상 징후 전파
      if (sensor.anomaly) {
        state.modules[s.m].anomaly = true;
        state.modules[s.m].types[s.t].anomaly = true;
      }
    });

    renderList();
    updateSelectedUI();
  };
}

function renderList() {
  const container = document.getElementById("moduleList");
  if (!container) return;

  // 리스트가 실시간으로 깜빡이는 것을 방지하기 위해 내용이 바뀔 때만 갱신하는 것이 좋으나
  // 우선은 구조적 정확성을 위해 초기화 후 생성합니다.
  container.innerHTML = "";

  let items = {};
  if (navigation.level === "module") items = state.modules;
  else if (navigation.level === "type") items = navigation.module.types;
  else items = navigation.type.sensors;

  Object.entries(items).forEach(([key, val]) => {
    const div = document.createElement("div");
    const isAnomaly = val.anomaly;

    div.className = `flex items-center justify-between px-4 py-3 cursor-pointer border-l-4 transition-all ${
      isAnomaly
        ? "bg-red-500/10 border-red-500 text-red-400"
        : "border-transparent hover:bg-white/5 text-[#e5e2e1]"
    }`;

    let label = "";
    if (navigation.level === "module")
      label = MODULE_NAMES[key] || `Module ${key}`;
    else if (navigation.level === "type")
      label = TYPE_NAMES[key] || `Type ${key}`;
    else label = `Sensor #${key & 0xff}`;

    div.innerHTML = `
            <div class="flex items-center gap-3">
                <span class="material-symbols-outlined text-sm">${isAnomaly ? "warning" : "adjust"}</span>
                <span>${label}</span>
            </div>
            <span class="material-symbols-outlined text-xs opacity-50">chevron_right</span>
        `;

    div.onclick = () => {
      if (navigation.level === "module") {
        navigation.level = "type";
        navigation.module = val;
        navigation.moduleKey = key;
      } else if (navigation.level === "type") {
        navigation.level = "sensor";
        navigation.type = val;
        navigation.typeKey = key;
      } else {
        selectedSensor = val;
      }
      renderList();
    };
    container.appendChild(div);
  });

  // 브레드크럼 업데이트
  let path = "ROOT";
  if (navigation.moduleKey) path += ` > ${MODULE_NAMES[navigation.moduleKey]}`;
  if (navigation.typeKey) path += ` > ${TYPE_NAMES[navigation.typeKey]}`;
  const bc = document.getElementById("breadcrumb");
  if (bc) bc.textContent = path;
}

function updateSelectedUI() {
  if (!selectedSensor) return;
  const last = selectedSensor.history[selectedSensor.history.length - 1];
  if (!last) return;

  // 데이터 주입
  const valEl = document.getElementById("currentValue");
  const avgEl = document.getElementById("currentAvg");
  const statEl = document.getElementById("currentStatus");
  const titleEl = document.querySelector("#currentSensor h2");

  if (valEl) valEl.textContent = last.value.toFixed(3);
  if (avgEl) avgEl.textContent = last.y.toFixed(3);
  if (statEl) {
    statEl.textContent = selectedSensor.anomaly ? "CRITICAL" : "OPERATIONAL";
    statEl.className = `text-xl font-bold mt-1 ${selectedSensor.anomaly ? "text-red-400" : "text-primary"}`;
  }
  if (titleEl)
    titleEl.textContent = `${MODULE_NAMES[selectedSensor.m]} / Sensor #${selectedSensor.id & 0xff}`;

  // 차트 업데이트
  chart.data.datasets = [
    {
      data: selectedSensor.history,
      borderColor: "#9ecaff",
      borderWidth: 2,
      pointRadius: 0,
      fill: true,
      backgroundColor: "rgba(158, 202, 255, 0.05)",
      tension: 0.4,
    },
  ];
  chart.update("none");
}

function setupBackButton() {
  const btn = document.getElementById("backBtn");
  if (btn) {
    btn.onclick = () => {
      if (navigation.level === "sensor") navigation.level = "type";
      else if (navigation.level === "type") navigation.level = "module";
      renderList();
    };
  }
}

main();
