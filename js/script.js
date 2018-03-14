/*
let imgUrls = [
  "img/lv0.png", // lv0
  "img/lv1.png", // lv1
  "img/lv2.png", // lv2
  "img/lv3.png", // lv3
  "img/error.png", // error
  "img/wait.png", // wait
];
*/
var baseUrl = "http://192.168.1.9/";
//var baseUrl = "/";

var imgUrls = [
  "https://image.ibb.co/kWdiwS/lv0.png", // lv0
  "https://image.ibb.co/gmuOwS/lv1.png", // lv1
  "https://image.ibb.co/c3vkAn/lv2.png", // lv2
  "https://image.ibb.co/mSxMi7/lv3.png", // lv3
  "https://image.ibb.co/duXyVn/error.png", // error
  "https://image.ibb.co/kAZ1i7/wait.png", // wait
];



$( document ).ready(function() {
  $("#alert").hide();
  $("#light").attr('src', imgUrls[5]);

  $("#wifiSettingsForm").submit(function(){
      $('#wifiModal').modal('hide');
      setWifiSettings($('#ssid').val(), $('#password').val());
      return false;
  });

  $("#mqttSettingsForm").submit(function(){
      $('#mqttModal').modal('hide');
      setMqttHost($('#mqtthost').val());
      return false;
  });

  $("#lv0").click(function(){
      setLevel(0);
  });

  $("#lv1").click(function(){
      setLevel(1);
  });

  $("#lv2").click(function(){
      setLevel(2);
  });

  $("#lv3").click(function(){
      setLevel(3);
  });

  $("#light").click(function(){
      toggleLamp();
  });

  $("#restart").click(function(){
      restartLamp();
  });

  $("#mqttbtn").click(function(){
      openMqttModal();
  });

  $.ajax({
    url: baseUrl + "lamp",
    success: function( data ) {
        $("#lv" + data.level).first().focus();
        changeImageLevel(data.level);
    },
    error: onError,
  });
});
function successAlert(msg) {
  $('#alert' ).removeClass( "alert-danger" );
  $('#alert' ).addClass( "alert-warning" );
  $('#alert').html("<strong>Success!</strong> " + msg)
  $("#alert").fadeTo(2000, 500).slideUp(500, function(){
      $("#alert").slideUp(500);
  });
}

function errorAlert(msg) {
  $('#alert' ).removeClass( "alert-warning" );
  $('#alert' ).addClass( "alert-danger" );
  $('#alert').html("<strong>Error!</strong> " + msg)
  $("#alert").fadeTo(2000, 500).slideUp(500, function(){
      $("#alert").slideUp(500);
  });
}

function getMqttHost() {
  $.ajax({
    type: "POST",
    url: baseUrl + "mqtthost",
    success: function( data ) {
        console.log(data);
    },
    error: onError,
  });
}

function openMqttModal() {
  $.ajax({
    url: baseUrl + "mqttstatus",
    success: function( data ) {
        $('#mqttHostTable').html(data.mqtt_host);
        $('#mqttEnableTable').html(data.mqtt_enabled);
        $('#mqttConnectedTable').html(data.mqtt_connected);
        $('#mqttModal').modal('show');
    },
    error: onError,
  });
}

function setLevel(lv) {
  $.ajax({
    type: "POST",
    url: baseUrl + "lamp",
    data: {"level": lv},
    success: function( data ) {
        $("#lv" + data.level).first().focus();
        changeImageLevel(data.level);
    },
    error: onError,
  });
}
function setMqttHost(hostname) {
  $.ajax({
    type: "POST",
    url: baseUrl + "mqtthost",
    data: {
      "mqtt_host": hostname,
    },
    success: function( data ) {
        console.log(data);
        successAlert("Mqtt host has been updated!");
    },
    error: onError,
  });
}

function setWifiSettings(ssid, password) {
  console.log({
    "ssid": ssid,
    "password": password
  });
  $.ajax({
    type: "POST",
    url: baseUrl + "wifisettings",
    data: {
      "ssid": ssid,
      "password": password
    },
    success: function( data ) {
        console.log(data);
        successAlert("Wifi settings have been updated!");
    },
    error: onError,
  });
}

function restartLamp() {
  $.ajax({
    url: baseUrl + "restart",
    success: function( data ) {
      location.reload();
    },
    error: onError,
  });
}

function toggleLamp() {
  $.ajax({
      url: baseUrl + "toggle",
      //url: "http://192.168.1.9/toggle",
    success: function( data ) {
        $("#lv" + data.level).first().focus();
        changeImageLevel(data.level);
    },
    error: onError,
  });
}


function changeImageLevel(lv) {
    $("#light")
        .fadeOut(200, function() {
            $("#light").attr('src', imgUrls[lv]);
        })
        .fadeIn(200);
}

function onError( obj, type, except ) {
  console.log(obj);
  console.log(obj.responseJSON.error);
  errorAlert(obj.responseJSON.error);
  changeImageLevel(4);
}
