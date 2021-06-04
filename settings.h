const char SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<head>
  <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
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
a{
  color: #FFF;
}
</style>
<body>
<div class="card">
  <a href="/">Home</a>
  <h1>Einstellungen</h1><br>
  <form method='POST' action='/set' enctype='multipart/form-data' id='settings_form'>
  Luftfeuchtigkeit
  <br>
  <input type='number' name='humidity'>
  <br>
  <br>
  <input type='submit' value='Update'>
  </form>
</div>
</body>
</html>
)=====";
