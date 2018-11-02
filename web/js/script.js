// var imgUrls = [
// "../img/lv0.png", // lv0
// "../img/lv1.png", // lv1
// "../img/lv2.png", // lv2
// "../img/lv3.png", // lv3
// "../img/error.png", // error
// "../img/wait.png", // wait
// ];

// var baseUrl = "http://192.168.1.4/";
let baseUrl = "/";

let imgUrls = [
  "https://image.ibb.co/iQuqUK/lv0.png", // lv0
  "https://image.ibb.co/dQkawz/lv1.png", // lv1
  "https://image.ibb.co/hYHc9K/lv2.png", // lv2
  "https://image.ibb.co/ngyjpK/lv3.png", // lv3
  "https://image.ibb.co/bC2c9K/error.png", // error
  "https://image.ibb.co/cJTjpK/wait.png", // wait
];

$(document).ready(function () {

  $("#settings").click(function () {
    $("html, body").animate({
      scrollTop: $(document).height()
    }, "slow");
    return false;
  });

  $("#alert").hide();
  $("#light").attr('src', imgUrls[5]);

  $("#wifiSettingsForm").submit(function () {
    $('#wifiModal').modal('hide');
    setWifiSettings($('#ssid').val(), $('#password').val());
    return false;
  });

  $("#mqttSettingsForm").submit(function () {
    $('#mqttModal').modal('hide');
    setMqttSettings({
      "mqtt_host": $('#mqtthost').val(),
      "mqtt_port": $('#mqttport').val(),
      "mqtt_username": $('#mqttusername').val(),
      "mqtt_password": $('#mqttpassword').val(),
    })
    return false;
  });
});

$("#lv0").click(function () {
  setLevel(0);
});

$("#lv1").click(function () {
  setLevel(1);
});

$("#lv2").click(function () {
  setLevel(2);
});

$("#lv3").click(function () {
  setLevel(3);
});

$("#light").click(function () {
  toggleLamp();
});

$("#restartbtn").click(function () {
  restartLamp();
});

$("#formatbtn").click(function () {
  formatLamp();
});

$("#mqttbtn").click(function () {
  openMqttModal();
});

$("#wifibtn").click(function () {
  openWifiModal();
});

$("#docsbtn").click(function () {
  location.href = '/docs';
});

$.ajax({
  url: baseUrl + "api/lamp",
  success: function (data) {
    $("#lv" + data.level).first().focus();
    changeImageLevel(data.level);
  },
  error: onError,
});

function successAlert(msg) {
  $('#alert').removeClass("alert-danger");
  $('#alert').addClass("alert-warning");
  $('#alert').html("<strong>Success!</strong> " + msg)
  $("#alert").fadeTo(2000, 500).slideUp(500, function () {
    $("#alert").slideUp(500);
  });
}

function createTable(id, data) {
  $("#" + id + " tbody tr").remove();
  for (key in data) {
    // check if the property/key is defined in the object itself, not in parent
    if (data.hasOwnProperty(key)) {
      if (key == "status") continue;
      tableAddRow(id, key, data[key]);
    }
  }
}

function tableAddRow(id, key, value) {
  $('#' + id + ' > tbody:last-child').append('<tr><td>' + key + '</td><td>' + value + '</td></tr>');
}

function errorAlert(msg) {
  $('#alert').removeClass("alert-warning");
  $('#alert').addClass("alert-danger");
  $('#alert').html("<strong>Error!</strong> " + msg)
  $("#alert").fadeTo(2000, 500).slideUp(500, function () {
    $("#alert").slideUp(500);
  });
}

function openMqttModal() {
  $.ajax({
    url: baseUrl + "api/mqtt",
    success: function (data) {
      createTable("mqttTable", data);
      $('#mqtthost').val(data.mqtt_host);
      $('#mqttport').val(data.mqtt_port);
      $('#mqttusername').val(data.mqtt_username);
      $('#mqttpassword').val(data.mqtt_password);
      $('#mqttModal').modal('show');
    },
    error: onError,
  });
}

function openWifiModal() {
  $.ajax({
    url: baseUrl + "api/wifi",
    success: function (data) {
      createTable("wifiTable", data);
      $('#ssid').val(data.ssid);
      $('#password').val(data.password);
      $('#wifiModal').modal('show');
    },
    error: onError,
  });
}


function setMqttSettings(data) {
  $.ajax({
    type: "POST",
    url: baseUrl + "api/mqtt",
    data: data,
    success: function (data) {
      successAlert("Mqtt host has been updated!");
    },
    error: onError,
  });
}

function setWifiSettings(ssid, password) {
  $.ajax({
    type: "POST",
    url: baseUrl + "api/wifi",
    data: {
      "ssid": ssid,
      "password": password
    },
    success: function (data) {
      successAlert("Wifi settings have been updated!");
    },
    error: onError,
  });
}

function restartLamp() {
  $.ajax({
    url: baseUrl + "api/restart",
    success: function (data) {
      location.reload();
    },
    error: onError,
  });
}

function formatLamp() {
  $.ajax({
    url: baseUrl + "api/format",
    success: function (data) {
      successAlert("Lamp formatted!");
    },
    error: onError,
  });
}

function setLevel(lv) {
  $.ajax({
    type: "POST",
    url: baseUrl + "api/lamp/set",
    data: {
      "level": lv
    },
    success: function (data) {
      $("#lv" + data.level).first().focus();
      changeImageLevel(data.level);
    },
    error: onError,
  });
}

function toggleLamp() {
  $.ajax({
    url: baseUrl + "api/lamp/toggle",
    success: function (data) {
      $("#lv" + data.level).first().focus();
      changeImageLevel(data.level);
    },
    error: onError,
  });
}


function changeImageLevel(lv) {
  $("#light")
    .fadeOut(200, function () {
      $("#light").attr('src', imgUrls[lv]);
    })
    .fadeIn(200);
}

function onError(obj, type, except) {
  if (obj.hasOwnProperty('error')) {
    errorAlert(obj.responseJSON.error);
  } else {
    console.log(obj);
  }
  changeImageLevel(4);
}