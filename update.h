const char UPDATE_page[] PROGMEM = R"=====(
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
  <h1>ESP32 Firmware Update</h1><br>
  <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
	<input type='file' name='update'>
	<input type='submit' value='Update'>
  </form>
  <div id='prg'>progress: 0%</div>
</div>
<script>
	$('form').submit(function(e) {
    e.preventDefault();
    var form = $('#upload_form')[0];
    var data = new FormData(form);
    $.ajax({
        url: '/upload',
        type: 'POST',
        data: data,
        contentType: false,
        processData: false,
        xhr: function() {
            var xhr = new window.XMLHttpRequest();
            xhr.upload.addEventListener('progress', function(evt) {
                if (evt.lengthComputable) {
                    var per = evt.loaded / evt.total;
                    $('#prg').html('progress: ' + Math.round(per * 100) + '%');
                }
            }, false);
            return xhr;
        },
        success: function(d, s) {
            console.log('success!')
            window.location = '/';
        },
        error: function(a, b, c) {}
    });
});
</script>
</body>
</html>
)=====";
