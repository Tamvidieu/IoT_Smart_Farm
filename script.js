import { initializeApp } from "https://www.gstatic.com/firebasejs/11.0.2/firebase-app.js";
import { getDatabase, ref, get, child, set } from "https://www.gstatic.com/firebasejs/11.0.2/firebase-database.js";

// Your web app's Firebase configuration
const firebaseConfig = {
    apiKey: "AIzaSyCF1XqTOmKN1nuoBb7YbWwb4gn_JENF-BU",
    authDomain: "smart-farm-bbb89.firebaseapp.com",
    databaseURL: "https://smart-farm-bbb89-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "smart-farm-bbb89",
    storageBucket: "smart-farm-bbb89.firebasestorage.app",
    messagingSenderId: "54401113090",
    appId: "1:54401113090:web:2375639783ed44fe389b1b",
    measurementId: "G-QXQFR73R0F"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

// Function to create a chart
function createChart(ctx, label, data) {
    console.log("Creating chart:", label, data);
    new Chart(ctx, {
        type: 'line',
        data: {
            labels: Object.keys(data),
            datasets: [{
                label: label,
                data: Object.values(data),
                borderColor: 'rgba(75, 192, 192, 1)',
                borderWidth: 1,
                fill: false
            }]
        },
        options: {
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Time'
                    },
                    ticks: {
                        maxTicksLimit: 5 // Giảm số lượng nhãn trên trục x
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: label
                    }
                }
            }
        }
    });
}

// Function to fetch data from Firebase
async function fetchData() {
    const dbRef = ref(database);
    console.log("Fetching data from Firebase...");
    try {
        const snapshot = await get(child(dbRef, 'history_data'));
        if (snapshot.exists()) {
            const historyData = snapshot.val();
            console.log("Fetched data:", historyData);
            const timestamps = Object.keys(historyData).sort().slice(-500); // Get last 100 timestamps
            console.log("Timestamps:", timestamps);
            const humidityData = {};
            const soilMoistureData = {};
            const temperatureData = {};
            const heaterData = {};
            const humidifierData = {};
            const pumpData = {};

            timestamps.forEach(timestamp => {
                const data = historyData[timestamp];
                if (data && data.sensors && data.peripheral) {
                    const date = new Date(parseInt(timestamp) * 1000);
                    const formattedDate = date.toLocaleString('en-GB', { 
                        day: '2-digit', month: '2-digit', hour: '2-digit', minute: '2-digit'
                    }).replace(',', ''); // Định dạng thời gian theo kiểu dd/MM HH:mm
                    humidityData[formattedDate] = data.sensors.humidity !== undefined ? data.sensors.humidity : null;
                    soilMoistureData[formattedDate] = data.sensors.soil_moisture !== undefined ? data.sensors.soil_moisture : null;
                    temperatureData[formattedDate] = data.sensors.temperature !== undefined ? data.sensors.temperature : null;
                    heaterData[formattedDate] = data.peripheral.heater !== undefined ? data.peripheral.heater : null;
                    humidifierData[formattedDate] = data.peripheral.humidifier !== undefined ? data.peripheral.humidifier : null;
                    pumpData[formattedDate] = data.peripheral.pump !== undefined ? data.peripheral.pump : null;
                } else {
                    console.warn("Incomplete data for timestamp:", timestamp);
                }
            });

            console.log("Humidity Data:", humidityData);
            console.log("Soil Moisture Data:", soilMoistureData);
            console.log("Temperature Data:", temperatureData);
            console.log("Heater Data:", heaterData);
            console.log("Humidifier Data:", humidifierData);
            console.log("Pump Data:", pumpData);

            // Create charts
            createChart(document.getElementById('humidityChart').getContext('2d'), 'Humidity', humidityData);
            createChart(document.getElementById('soilMoistureChart').getContext('2d'), 'Soil Moisture', soilMoistureData);
            createChart(document.getElementById('temperatureChart').getContext('2d'), 'Temperature', temperatureData);
            createChart(document.getElementById('heaterChart').getContext('2d'), 'Heater', heaterData);
            createChart(document.getElementById('humidifierChart').getContext('2d'), 'Humidifier', humidifierData);
            createChart(document.getElementById('pumpChart').getContext('2d'), 'Pump', pumpData);
        } else {
            console.log("No data available");
        }
    } catch (error) {
        console.error("Error fetching data:", error);
    }
}

// Function to save trigger values to Firebase
async function saveTriggerValues() {
    const humidityTrigger = document.getElementById('humidityTrigger').value;
    const soilMoistureTrigger = document.getElementById('soilMoistureTrigger').value;
    const temperatureTrigger = document.getElementById('temperatureTrigger').value;
    
    console.log("Saving trigger values to Firebase...", { humidityTrigger, soilMoistureTrigger, temperatureTrigger });
    
    try {
        await set(ref(database, 'realtime_data/trigger'), {
            humidity: humidityTrigger,
            soil_moisture: soilMoistureTrigger,
            temperature: temperatureTrigger
        });
        console.log("Trigger values saved successfully");
        alert("Trigger values saved successfully");
    } catch (error) {
        console.error("Error saving trigger values:", error);
        alert("Error saving trigger values");
    }
}

// Fetch data when the page loads
window.onload = function() {
    console.log("Window loaded, fetching data...");
    fetchData();
    setInterval(fetchData, 5000); // Update data every 5 seconds

    // Add event listener for save button
    document.getElementById('saveTriggerBtn').addEventListener('click', saveTriggerValues);
};
