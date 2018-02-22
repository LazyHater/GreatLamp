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
let imgUrls = [
  "https://image.ibb.co/kWdiwS/lv0.png", // lv0
  "https://image.ibb.co/gmuOwS/lv1.png", // lv1
  "https://image.ibb.co/c3vkAn/lv2.png", // lv2
  "https://image.ibb.co/mSxMi7/lv3.png", // lv3
  "https://image.ibb.co/duXyVn/error.png", // error
  "https://image.ibb.co/kAZ1i7/wait.png", // wait
];

//let baseUrl = "http://192.168.1.8/";
let baseUrl = "/";

$( document ).ready(function() {
  $.ajax({
    url: baseUrl + "lamp",
    success: function( data ) {
        $("#lv" + data.level).first().focus();
        changeImageLevel(data.level);
    },
    error: onError,
  });
});

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

function setSettings(ssid, password) {
  console.log({
    "ssid": ssid,
    "password": password
  });
  $.ajax({
    type: "POST",
    url: baseUrl + "settings",
    data: {
      "ssid": ssid,
      "password": password
    },
    success: function( data ) {
        console.log(data);
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
  console.log(obj, type, except);
  changeImageLevel(4);
}
