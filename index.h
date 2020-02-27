const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
	<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
</head>
<style>
.card{
    max-width: 400px;
     min-height: 250px;
     background: #02b875;
     padding: 30px;
     box-sizing: border-box;
     color: #FFF;
     margin:20px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
}
</style>
<body>
<div class="card">
  <h1>ESP32 Main Page</h1>
  <h4>L&uuml;fter an: <span id="fan_on"></span></h4>
  <h4>Temperatur: <span id="temperature">0%</span></h4>
  <h4>Luftfeuchtigkeit: <span id="humidity">0%</span></h4>
  <h4>Barometrischer Druck: <span id="pressure">0</span></h4>
  <h4>H&ouml;henmeter: <span id="height">0</span></h4>
  <button type="button" id="activate">L&uuml;fter starten</button>
  <button type="button" id="reset">L&uuml;fter Reset</button>
  <br><br><a href="/update">Firmware Update</a>
</div>
<script>

setInterval(function() {
  getData();
}, 1000);

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200)
    {
	    var data = this.responseText.split(",");
      document.getElementById("temperature").innerHTML = data[0];
	    document.getElementById("humidity").innerHTML = data[1];
      document.getElementById("pressure").innerHTML = data[2];
      document.getElementById("height").innerHTML = data[3];

      var fanValue = data[4];
      if(fanValue === "true")
      {
        document.getElementById("fan_on").innerHTML = "Ja " + data[5] + " Sekunden verbleiben";
      }
      else
      {
        document.getElementById("fan_on").innerHTML = "Nein";
      }
    }
  };
  xhttp.open("GET", "read_data", true);
  xhttp.send();
}

$(function()
{
  $('#activate').click(function()
  {
    $.ajax({
      url: '/activate_fan',
      type: 'POST'
      });
  });

  $('#reset').click(function()
  {
    $.ajax({
      url: '/reset_fan',
      type: 'POST'
      });
  });
});
</script>
</body>
</html>
)=====";
