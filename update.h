const char UPDATE_page[] PROGMEM = R"=====(
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
        },
        error: function(a, b, c) {}
    });
});
</script>
</body>
</html>
)=====";
