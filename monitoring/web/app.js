// =============================
// 상태
// =============================
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

// =============================
// 이름 매핑
// =============================
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
  6: "Motor Voltage",
  7: "Motor Speed",
  8: "Vibration",
  9: "Vacuum Pressure",
  10: "Flow",
  11: "Vision Alignment",
  12: "Defect Detection",
  13: "Humidity",
  14: "Airflow",
  15: "Power Consumption",
};

// =============================
// main
// =============================
function main() {
  initChart();
  setupBackButton();
  connectWebSocket();
}

// =============================
// WebSocket
// =============================
function connectWebSocket() {
  const ws = new WebSocket("ws://localhost:8080");

  ws.onmessage = (e) => handleMessage(e.data);
}

// =============================
// 데이터 처리
// =============================
function handleMessage(data) {
  const json = JSON.parse(data);

  resetAnomalyFlags();
  updateState(json);
  renderModuleList();
  updateSelectedSensorChart();
}

function updateState(json) {
  json.sensors.forEach(updateSensor);
}

function updateSensor(sensor) {
  const { id, m, t, avg, anomaly } = sensor;

  const module = getOrCreateModule(m);
  const type = getOrCreateType(module, t);
  const s = getOrCreateSensor(type, id);

  s.id = id;
  s.m = m;
  s.t = t;

  updateSensorHistory(s, avg);
  updateAnomaly(s, type, module, anomaly);
}

// =============================
// 상태 생성
// =============================
function getOrCreateModule(m) {
  if (!state.modules[m]) {
    state.modules[m] = { types: {}, anomaly: false };
  }
  return state.modules[m];
}

function getOrCreateType(module, t) {
  if (!module.types[t]) {
    module.types[t] = { sensors: {}, anomaly: false };
  }
  return module.types[t];
}

function getOrCreateSensor(type, id) {
  if (!type.sensors[id]) {
    type.sensors[id] = { history: [], anomaly: false };
  }
  return type.sensors[id];
}

// =============================
// 데이터 처리
// =============================
function updateSensorHistory(sensor, avg) {
  sensor.history.push({
    x: Date.now(),
    y: avg,
    anomaly: sensor.anomaly,
  });

  if (sensor.history.length > 100) {
    sensor.history.shift();
  }
}

function updateAnomaly(sensor, type, module, anomaly) {
  sensor.anomaly = anomaly === 1;

  if (sensor.anomaly) {
    type.anomaly = true;
    module.anomaly = true;
    addAlert(sensor);
  }
}

function resetAnomalyFlags() {
  for (let m in state.modules) {
    state.modules[m].anomaly = false;

    for (let t in state.modules[m].types) {
      state.modules[m].types[t].anomaly = false;
    }
  }
}

// =============================
// UI 렌더링
// =============================
function renderModuleList() {
  navigation.level = "module";
  navigation.moduleKey = null;
  navigation.typeKey = null;

  updateBreadcrumb();

  const container = document.getElementById("moduleList");
  container.innerHTML = "";

  for (let m in state.modules) {
    const el = createModuleElement(m, state.modules[m]);
    container.appendChild(el);
  }
}

function createModuleElement(m, module) {
  const div = document.createElement("div");
  div.className = "item";
  if (module.anomaly) div.classList.add("anomaly");

  div.textContent = MODULE_NAMES[m] || m;

  div.onclick = () => {
    navigation.level = "type";
    navigation.moduleKey = m;
    navigation.module = module;

    renderTypeList(module);
    updateBreadcrumb();
  };

  return div;
}

function renderTypeList(module) {
  const container = document.getElementById("moduleList");
  container.innerHTML = "";

  for (let t in module.types) {
    container.appendChild(createTypeElement(t, module.types[t]));
  }
}

function createTypeElement(t, type) {
  const div = document.createElement("div");
  div.className = "item";
  if (type.anomaly) div.classList.add("anomaly");

  div.textContent = TYPE_NAMES[t] || t;

  div.onclick = () => {
    navigation.level = "sensor";
    navigation.typeKey = t;
    navigation.type = type;

    renderSensorList(type);
    updateBreadcrumb();
  };

  return div;
}

function renderSensorList(type) {
  const container = document.getElementById("moduleList");
  container.innerHTML = "";

  for (let id in type.sensors) {
    container.appendChild(createSensorElement(id, type.sensors[id]));
  }
}

function createSensorElement(id, sensor) {
  const div = document.createElement("div");
  div.className = "item";

  if (sensor.anomaly) div.classList.add("anomaly");
  if (sensor === selectedSensor) div.classList.add("selected");

  div.textContent = `#${id & 0xff}`;
  div.onclick = () => selectSensor(sensor);

  return div;
}

// =============================
// navigation
// =============================
function setupBackButton() {
  document.getElementById("backBtn").onclick = handleBack;
}

function handleBack() {
  if (navigation.level === "sensor") {
    navigation.level = "type";
    renderTypeList(navigation.module);
  } else if (navigation.level === "type") {
    renderModuleList();
  }

  updateBreadcrumb();
}

function updateBreadcrumb() {
  const el = document.getElementById("breadcrumb");

  let text = "Modules";

  if (navigation.moduleKey !== null) {
    text += ` > ${MODULE_NAMES[navigation.moduleKey]}`;
  }

  if (navigation.typeKey !== null) {
    text += ` > ${TYPE_NAMES[navigation.typeKey]}`;
  }

  el.textContent = text;
}

// =============================
// 센서 선택
// =============================
function selectSensor(sensor) {
  selectedSensor = sensor;

  updateChart(sensor);
  updateSensorInfo(sensor);
}

// =============================
// Chart
// =============================
function initChart() {
  const ctx = document.getElementById("chart").getContext("2d");

  chart = new Chart(ctx, {
    type: "line",
    data: { datasets: [] },
    options: { animation: false },
  });
}

function updateChart(sensor) {
  chart.data.datasets = [
    {
      data: sensor.history,
      borderColor: "blue",
      pointBackgroundColor: sensor.history.map((p) =>
        p.anomaly ? "red" : "blue",
      ),
    },
  ];

  chart.update("none");
}

function updateSelectedSensorChart() {
  if (!selectedSensor) return;

  updateChart(selectedSensor);
  updateSensorInfo(selectedSensor);
}

// =============================
// 센서 정보
// =============================
function updateSensorInfo(sensor) {
  const last = sensor.history[sensor.history.length - 1];
  if (!last) return;

  document.getElementById("currentValue").textContent = last.y.toFixed(3);
  document.getElementById("currentAvg").textContent = last.y.toFixed(3);

  const status = document.getElementById("currentStatus");

  if (last.anomaly) {
    status.textContent = "ANOMALY";
    status.classList.add("anomaly-text");
  } else {
    status.textContent = "NORMAL";
    status.classList.remove("anomaly-text");
  }
}

// =============================
// Alert → 이동
// =============================
function addAlert(sensor) {
  const container = document.getElementById("alerts");

  const div = document.createElement("div");
  div.className = "item anomaly";

  div.textContent = `⚠ ${MODULE_NAMES[sensor.m]} / ${TYPE_NAMES[sensor.t]} / #${sensor.id & 0xff}`;

  div.onclick = () => navigateToSensor(sensor);

  container.prepend(div);

  if (container.children.length > 10) {
    container.removeChild(container.lastChild);
  }
}

function navigateToSensor(sensor) {
  const module = state.modules[sensor.m];
  const type = module.types[sensor.t];
  const target = type.sensors[sensor.id];

  navigation.level = "sensor";
  navigation.moduleKey = sensor.m;
  navigation.typeKey = sensor.t;
  navigation.module = module;
  navigation.type = type;

  renderSensorList(type);
  updateBreadcrumb();

  selectSensor(target);
}

// =============================
// 실행
// =============================
main();
