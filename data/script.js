// Update interval
const AIR_QUALITY_SENSOR_UPDATE_SECONDS = 60;

const AQI_INTERVAL_TAGS = ["aqi_current", "aqi_10min", "aqi_1hour", "aqi_24hour"];
const ENV_INTERVAL_TAGS = ["temperature_f", "pressure", "humidity"];

// DOM node mappings for metric value and color
const aqiValueDivs = AQI_INTERVAL_TAGS.reduce(
  (acc, tag) => ({ ...acc, [tag]: document.getElementById(`value-${tag}`) }),
  {},
);
const aqiBgDivs = AQI_INTERVAL_TAGS.reduce(
  (acc, tag) => ({ ...acc, [tag]: document.getElementById(`bg-${tag}`) }),
  {},
);
const envValueDivs = ENV_INTERVAL_TAGS.reduce(
  (acc, tag) => ({ ...acc, [tag]: document.getElementById(`value-${tag}`) }),
  {},
);

function showAQI(evt, avr_window) {
  const tabcontents = document.getElementsByClassName("aqi-display");
  for (const tabcontent of tabcontents) {
    tabcontent.style.display = "none";
  }
  const tablinks = document.getElementsByClassName("aqi_window_option");
  for (const tablink of tablinks) {
    tablink.classList.remove("active");
  }
  document.getElementById(avr_window).style.display = "block";
  evt.currentTarget.classList.add("active");
}

function colorToAQIClass(color) {
  return "aqi-" + color;
}

async function fetchMetrics() {
  // Response format:
  // {
  //   air_quality_index: {
  //     aqi_current: {
  //       value: float,
  //       color: string,
  //     },
  //     aqi_10min: { ... },
  //     aqi_1hour: { ... },
  //     aqi_24hour: { ... },
  //   },
  //   has_environment_sensor: true/false
  //   environment?: {
  //     temperature: { value: float },
  //     temperature_f: { value: float },
  //     pressure: { value: float },
  //     humidity: { value: float },
  //   },
  // }
  return await fetch("/json").then(r => r.json());
}

async function updateMetrics() {
  const response = await fetchMetrics();
  const aqiMetrics = response.air_quality_index;
  for (const [tag, valueDiv] of Object.entries(aqiValueDivs)) {
    valueDiv.textContent = (aqiMetrics[tag].value || 0).toFixed(1);
  }
  for (const [tag, bgDiv] of Object.entries(aqiBgDivs)) {
    bgDiv.className = colorToAQIClass(aqiMetrics[tag].color);
  }

  if (response.has_environment_sensor) {
    const envMetrics = response.environment;
    for (const [tag, valueDiv] of Object.entries(envValueDivs)) {
      valueDiv.textContent = (envMetrics[tag].value || 0).toFixed(1);
    }
  } else {
    document.getElementById("bme680").style.display = "none";
  }

  document.getElementById("sensor_name").textContent = response.sensor_id;
  document.getElementById("content").style.display = "block";
}

function main() {
  // Get the element with id="defaultOpen" and click on it
  document.getElementById("defaultOpen").click();

  // Load the initial metrics
  updateMetrics();

  // Start polling loop
  setInterval(updateMetrics, (AIR_QUALITY_SENSOR_UPDATE_SECONDS || 60) * 1000);
}

main();
