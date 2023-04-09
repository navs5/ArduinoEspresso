#ifndef _WEB_SERVER_PAGES_H_
#define _WEB_SERVER_PAGES_H_

namespace WebServerHtml
{
 
/*
 * Updater Page
 */
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>Espresso Firmware Update Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td><input type='file' name='update'></td>"
            "<td><input type='submit' onclick='check(this.form)' value='Update'></td>"
        "</tr>"
        "</tr>"
            "<td><div id='prg'>progress: 0%</div></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
        "$('form').submit(function(e){"
        "e.preventDefault();"
        "var form = $('#upload_form')[0];"
        "var data = new FormData(form);"
        " $.ajax({"
        "url: '/update',"
        "type: 'POST',"
        "data: data,"
        "contentType: false,"
        "processData:false,"
        "xhr: function() {"
        "var xhr = new window.XMLHttpRequest();"
        "xhr.upload.addEventListener('progress', function(evt) {"
        "if (evt.lengthComputable) {"
        "var per = evt.loaded / evt.total;"
        "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
        "}"
        "}, false);"
        "return xhr;"
        "},"
        "success:function(d, s) {"
        "console.log('success!')" 
        "},"
        "error: function (a, b, c) {"
        "}"
        "});"
        "});"
    "}"
"</script>"
"<script>"

"</script>";

 } // namespace webServerHtml

 #endif  // _WEB_SERVER_PAGES_H_