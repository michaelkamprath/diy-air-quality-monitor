// Update interval set to same value in Configuration.h
const AIR_QUALITY_SENSOR_UPDATE_SECONDS = ^SENSOR-UPDATE-SECONDS^;
const INTERVAL_TAGS = ["current", "tenMinutes", "hour", "day"];

// DOM node mappings for metric value and color
const valueDivs = INTERVAL_TAGS.reduce(
  (acc, tag) => ({ ...acc, [tag]: document.getElementById(`value-${tag}`) }),
  {},
);
const bgDivs = INTERVAL_TAGS.reduce(
  (acc, tag) => ({ ...acc, [tag]: document.getElementById(`bg-${tag}`) }),
  {},
);

function showAQI(evt, avr_window) {
  var i, tabcontent, tablinks;
  tabcontent = document.getElementsByClassName("aqi-display");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }
  tablinks = document.getElementsByClassName("aqi_window_option");
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].className = tablinks[i].className.replace(" active", "");
  }
  document.getElementById(avr_window).style.display = "block";
  evt.currentTarget.className += " active";
}

function getAQIStatusColor(aqiValue) {
  if (aqiValue <= 50) {
    return 'aqi-green';
  } else if (aqiValue <= 100) {
    return 'aqi-yellow';
  } else if (aqiValue <= 150) {
    return 'aqi-orange';
  } else if (aqiValue <= 200) {
    return 'aqi-red';
  } else if (aqiValue <= 300) {
    return 'aqi-purple';
  } else {
    return 'aqi-maroon';
  }
}

async function fetchMetrics() {
  // Response format:
  // {
  //   current: Number,
  //   tenMinutes: Number,
  //   hour: Number,
  //   day: Number,
  // }
  return await fetch("/json").then(r => r.json());
}

async function updateMetrics() {
  const metrics = await fetchMetrics();
  for (const [tag, valueDiv] of Object.entries(valueDivs)) {
    valueDiv.textContent = (metrics[tag] || 0).toFixed(1);
  }
  for (const [tag, bgDiv] of Object.entries(bgDivs)) {
    bgDiv.className = getAQIStatusColor(metrics[tag] || 0);
  }
}

function main() {
  // Get the element with id="defaultOpen" and click on it
  document.getElementById("defaultOpen").click();
  // Start polling loop
  setInterval(updateMetrics, (AIR_QUALITY_SENSOR_UPDATE_SECONDS || 1) * 1000);
}

main();
