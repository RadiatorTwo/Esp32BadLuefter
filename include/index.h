const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<head>
  <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js'></script>
</head>
<style>
.card{
  background: #555555;
  padding: 30px;
  box-sizing: border-box;
  color: #FFF;
  margin: auto;
  box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
  width: 100%;
  position:fixed;
  top:0;
  bottom:0;
  left:0;
  right:0;
}
html{
    width:100%;
    height:100%;
}
body{
  width:100%;
  height:100%;
  background-color:#DDD;
  text-align: center;
}
button{
  height: 50px;
  width: 50%;
  background-color: #555555;
  border: 2px solid #FFF;
  color: white;
  padding: 15px 32px;
  margin: 10px auto;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  border-radius: 4px;
  transition-duration: 0.4s;
  display: block;
  box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);
}
button:hover {
  background-color: #aaa;
  color: white;
}
a{
  color: #FFF;
}
</style>
<body>
<div class="card">
  <h1>ESP32 Main Page</h1>
  <h4>L&uuml;fter an: <span id="fan_on"></span></h4>
  <h4><span id="remaining_time"></span></h4>
  <h4>Temperatur: <span id="temperature">0 °C</span></h4>
  <h4>Luftfeuchtigkeit: <span id="humidity">0 %</span></h4>
  <h4>Taupunkt: <span id="dewpoint">0 °C</span></h4>
  <h4>Aktuelle Taupunkt Differenz: <span id="dewpointdiff">0 °C</span></h4>
  <h4>Schwellwert Taupunkt Differenz: <span id="maxdewpoint">0 °C</span></h4>
  <h4>Barometrischer Druck: <span id="pressure">0</span></h4>
  <h4>H&ouml;henmeter: <span id="height">0</span></h4>
  <button type="button" id="activate">L&uuml;fter starten</button>
  <button type="button" id="reset">L&uuml;fter Reset</button>
  <button type="button" id="long_run">60 minuten</button>
  <br><br><a href="/settings">Einstellungen</a>
  <br><br><a href="/update">Firmware Update</a>
  <br><br><div>Version: 0.83</div>
</div>
<script>

setInterval(function() {
  getData();
}, 250);

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200)
    {
      var data = this.responseText.split(",");
      document.getElementById("temperature").innerHTML = data[0];
      document.getElementById("humidity").innerHTML = data[1];
      document.getElementById("dewpoint").innerHTML = data[2];
      document.getElementById("pressure").innerHTML = data[3];
      document.getElementById("height").innerHTML = data[4];

      var fanValue = data[5];
      if(fanValue === "true")
      {
        document.getElementById("fan_on").innerHTML = "Ja";
        document.getElementById("remaining_time").innerHTML = data[6];
      }
      else
      {
        document.getElementById("fan_on").innerHTML = "Nein";
        document.getElementById("remaining_time").innerHTML = "";
      }

      document.getElementById("maxdewpoint").innerHTML = data[7];
      document.getElementById("dewpointdiff").innerHTML = data[8];
    }
  };
  xhttp.open("GET", "read_data", true);
  xhttp.send();
}

$(function() {
  $('#activate').click(function() {
    $.ajax({
      url: '/activate_fan',
      type: 'POST',
      data: {}, // Leeres Datenobjekt hinzugefügt
      success: function(response) {
        console.log('Lüfter wurde aktiviert.');
      },
      error: function(xhr, status, error) {
        console.error('Fehler beim Aktivieren des Lüfters:', error);
        console.error('Status:', status);
        console.error('Antwort:', xhr.responseText);
      }
    });
  });

  $('#reset').click(function() {
    $.ajax({
      url: '/reset_fan',
      type: 'POST',
      data: {},
      success: function(response) {
        console.log('Lüfter wurde zurückgesetzt.');
      },
      error: function(xhr, status, error) {
        console.error('Fehler beim Zurücksetzen des Lüfters:', error);
        console.error('Status:', status);
        console.error('Antwort:', xhr.responseText);
      }
    });
  });

  $('#long_run').click(function() {
    $.ajax({
      url: '/longrun_fan',
      type: 'POST',
      data: {},
      success: function(response) {
        console.log('Lüfter läuft für 60 Minuten.');
      },
      error: function(xhr, status, error) {
        console.error('Fehler beim Starten des Langlaufs:', error);
        console.error('Status:', status);
        console.error('Antwort:', xhr.responseText);
      }
    });
  });
});
</script>
</body>
</html>
)=====";
