#include "web_ui.h"
#include "config.h"

WebUI::WebUI(
    VoltageSensor &sensor,
    Pump &pump,
    Wiper &wiper,
    Cleaner &cleaner,
    AutoCleanManager &autoClean
)
    : _sensor(sensor),
      _pump(pump),
      _wiper(wiper),
      _cleaner(cleaner),
      _autoClean(autoClean),
      _server(WEB_PORT) {
}

void WebUI::begin() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    _server.on("/api/wipe", HTTP_POST, [this]() { handleWipe(); });
    _server.on("/api/dispense", HTTP_POST, [this]() { handleDispense(); });
    _server.on("/api/clean", HTTP_POST, [this]() { handleClean(); });
    _server.on("/api/baseline", HTTP_POST, [this]() { handleSetBaseline(); });
    _server.on("/api/auto", HTTP_POST, [this]() { handleAutoToggle(); });

    _server.begin();
}

void WebUI::update() {
    _server.handleClient();
}

void WebUI::handleStatus() {
    String json = "{";

    json += "\"voltage\":" + String(_sensor.getVoltage(), 3);
    json += ",\"adcVoltage\":" + String(_sensor.getADCVoltage(), 3);
    json += ",\"rawADC\":" + String(_sensor.getRawADC());

    json += ",\"baseline\":" + String(_autoClean.getBaseline(), 3);
    json += ",\"dropPercent\":" + String(_autoClean.getDropPercent(), 2);

    json += ",\"pump\":";
    json += _pump.isRunning() ? "true" : "false";

    json += ",\"wiping\":";
    json += _wiper.isRunning() ? "true" : "false";

    json += ",\"cleaning\":";
    json += _cleaner.isRunning() ? "true" : "false";

    json += ",\"autoEnabled\":";
    json += _autoClean.isEnabled() ? "true" : "false";

    json += ",\"lightCondition\":";
    json += _autoClean.isLightConditionLocked() ? "true" : "false";

    json += ",\"servoAngle\":" + String(_wiper.getCurrentAngle());

    json += ",\"preCleanVoltage\":" +
            String(_autoClean.getPreCleanVoltage(), 3);

    json += ",\"postCleanVoltage\":" +
            String(_autoClean.getPostCleanVoltage(), 3);

    json += ",\"improvementPercent\":" +
            String(_autoClean.getLastImprovementPercent(), 2);

    json += ",\"state\":\"";
    json += _autoClean.getStateText();
    json += "\"";

    json += "}";

    _server.send(200, "application/json", json);
}

void WebUI::handleWipe() {
    uint8_t count = 1;

    if (_server.hasArg("count")) {
        count = constrain(_server.arg("count").toInt(), 1, 20);
    }

    _wiper.wipe(count);
    _server.send(200, "text/plain", "Wipe started");
}

void WebUI::handleDispense() {
    uint32_t duration = DEFAULT_DISPENSE_TIME_MS;

    if (_server.hasArg("ms")) {
        duration = constrain(
            _server.arg("ms").toInt(),
            100,
            (int)MAX_PUMP_RUNTIME_MS
        );
    }

    _pump.dispense(duration);
    _server.send(200, "text/plain", "Dispensing");
}

void WebUI::handleClean() {
    _cleaner.clean(
        DEFAULT_WIPE_COUNT,
        DEFAULT_DISPENSE_TIME_MS
    );

    _server.send(200, "text/plain", "Cleaning started");
}

void WebUI::handleSetBaseline() {
    _autoClean.setBaselineFromCurrentVoltage();
    _server.send(200, "text/plain", "Baseline updated");
}

void WebUI::handleAutoToggle() {
    bool enabled = !_autoClean.isEnabled();

    if (_server.hasArg("enabled")) {
        enabled = _server.arg("enabled") == "1" ||
                  _server.arg("enabled") == "true";
    }

    _autoClean.setEnabled(enabled);
    _server.send(200, "text/plain", enabled ? "Auto ON" : "Auto OFF");
}

void WebUI::handleRoot() {
static const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Solar Cleaner</title>
<style>
*{box-sizing:border-box}
body{
  margin:0;
  font-family:Arial,sans-serif;
  background:#0c111b;
  color:#fff
}
.container{
  max-width:980px;
  margin:auto;
  padding:24px
}
h1{margin-bottom:4px}
.subtitle{
  color:#8892a6;
  margin-bottom:24px
}
.grid{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(180px,1fr));
  gap:14px
}
.card{
  background:#151c29;
  border:1px solid #273142;
  border-radius:14px;
  padding:18px
}
.label{
  color:#8993a8;
  font-size:12px;
  letter-spacing:.4px
}
.value{
  font-size:30px;
  margin-top:8px;
  font-weight:bold
}
.green{color:#4ee39a}
.orange{color:#ffbd66}
.red{color:#ff6b6b}
.controls{margin-top:20px}
button{
  border:none;
  padding:13px 16px;
  margin:6px 6px 6px 0;
  border-radius:10px;
  font-size:14px;
  cursor:pointer;
  background:#2f71ff;
  color:white
}
button:hover{opacity:.88}
.clean{background:#19a974}
.pump{background:#9254de}
.warning{
  margin-top:16px;
  border:1px solid #6c5525;
  background:#2a2417;
  color:#ffd27a;
  padding:12px;
  border-radius:10px;
  display:none
}
.state{
  font-size:14px;
  color:#aeb7c8;
  margin-top:12px
}
.status{
  height:10px;
  width:10px;
  display:inline-block;
  border-radius:50%;
  background:#555;
  margin-right:7px
}
.active{
  background:#42e893;
  box-shadow:0 0 8px #42e893
}
</style>
</head>
<body>
<div class="container">

<h1>☀ Solar Cleaner</h1>
<div class="subtitle">ESP32-S3 autonomous panel maintenance</div>

<div class="grid">

<div class="card">
<div class="label">PANEL VOLTAGE</div>
<div id="voltage" class="value green">--</div>
</div>

<div class="card">
<div class="label">BASELINE</div>
<div id="baseline" class="value">--</div>
</div>

<div class="card">
<div class="label">VOLTAGE DROP</div>
<div id="drop" class="value orange">--</div>
</div>

<div class="card">
<div class="label">SERVO POSITION</div>
<div id="servo" class="value">--</div>
</div>

<div class="card">
<div class="label">CLEANING IMPROVEMENT</div>
<div id="improvement" class="value">--</div>
</div>

</div>

<div id="lightingWarning" class="warning">
⚠ Low output did not improve after cleaning. Auto-cleaning is locked
until the voltage recovers, because the system currently infers
cloud/shade/light conditions rather than dirt.
</div>

<div class="controls card">
<h3>Manual Controls</h3>

<button onclick="wipe(1)">Wipe ×1</button>
<button onclick="wipe(3)">Wipe ×3</button>
<button class="pump" onclick="dispense()">Dispense</button>
<button class="clean" onclick="cleanPanel()">💧 Clean Panel</button>
<button onclick="setBaseline()">Set Current as Baseline</button>
<button onclick="toggleAuto()">Toggle Auto Clean</button>

<hr style="border-color:#273142;margin:20px 0">

<p><span id="pumpIndicator" class="status"></span>Pump</p>
<p><span id="wipeIndicator" class="status"></span>Wiper</p>
<p><span id="cleanIndicator" class="status"></span>Cleaning sequence</p>
<p><span id="autoIndicator" class="status"></span>Auto clean</p>

<div id="state" class="state">State: --</div>
<div id="adc" class="state">ADC: --</div>

</div>
</div>

<script>
let lastStatus = null;

async function updateStatus(){
  try{
    const r = await fetch("/api/status");
    const d = await r.json();
    lastStatus = d;

    document.getElementById("voltage").innerText =
      d.voltage.toFixed(3) + " V";

    document.getElementById("baseline").innerText =
      d.baseline.toFixed(3) + " V";

    document.getElementById("drop").innerText =
      d.dropPercent.toFixed(1) + " %";

    document.getElementById("servo").innerText =
      d.servoAngle + "°";

    document.getElementById("improvement").innerText =
      d.improvementPercent.toFixed(1) + " %";

    document.getElementById("state").innerText =
      "State: " + d.state;

    document.getElementById("adc").innerText =
      "ADC pin: " + d.adcVoltage.toFixed(3) +
      " V | Raw: " + d.rawADC;

    setIndicator("pumpIndicator", d.pump);
    setIndicator("wipeIndicator", d.wiping);
    setIndicator("cleanIndicator", d.cleaning);
    setIndicator("autoIndicator", d.autoEnabled);

    document.getElementById("lightingWarning").style.display =
      d.lightCondition ? "block" : "none";
  }catch(e){
    console.log(e);
  }
}

function setIndicator(id,state){
  const e=document.getElementById(id);
  if(state) e.classList.add("active");
  else e.classList.remove("active");
}

function wipe(count){
  fetch("/api/wipe?count="+count,{method:"POST"});
}
function dispense(){
  fetch("/api/dispense?ms=1200",{method:"POST"});
}
function cleanPanel(){
  fetch("/api/clean",{method:"POST"});
}
function setBaseline(){
  fetch("/api/baseline",{method:"POST"});
}
function toggleAuto(){
  fetch("/api/auto",{method:"POST"});
}

setInterval(updateStatus,500);
updateStatus();
</script>
</body>
</html>
)rawliteral";

    _server.send_P(200, "text/html", PAGE);
}
