var x = 0;

let temperature,dustLevel
var humidity = 50 

client = new Paho.MQTT.Client("mqtt.netpie.io",443,"d88bd0e4-0a50-4c03-bd82-9aaa6c4bc94e"); // The last one is Client ID
client.onMessageArrived = onMessageArrived;

var options = {
    useSSL: true,
    userName: "78Rg1vZ1uGQz3yd3UkAxB3PBJnnB7gFH", // This is token
    password: "XYcqPXeSJBJ2NWHLL1U2qe5u6JQcrbt2", // This is secret
    onSuccess: onConnect,
    onFailure: doFail,                                                  
}



client.connect(options);

function onConnect(){
    client.subscribe("@msg/test");
}

function doFail(e){
    console.log(e);
}

function toggleStatus()
{
    // เป็นปุ่มที่ใช้เปิดปิดน้ำ
    var button = document.getElementById("my-button");
    if (button.innerHTML === "off") {
        setStatus("on")
        mqttSend("@msg/on", "on");
    } else {
        setStatus("off")   
        mqttSend("@msg/on", "off");
    }
    console.log(button.innerHTML);
    // onMessageArrived("")
}

function setStatus(status) {
    var button = document.getElementById("my-button");
    if (status == "on") {
        button.innerHTML = "on";
        button.classList.add("on"); // เพิ่มคลาส 'on' เพื่อเปลี่ยนสีเป็นสีเขียว
    }

    else {
        button.innerHTML = "off";
        button.classList.remove("on"); // ลบคลาส 'on' เพื่อกลับไปสีแดง
    }
}



function onMessageArrived(message)
{
    // ใช้ array ขนาด 3 ช่อง test ก่อนก็ได้นะแบบเสมือนรับข้อมูลจาก Sensor มา ยังไม่ต้องส่งข้อมูลจริงๆ
    // เราน่าจะรับมาเป็น String รูปแบบเป็น temp,humi,dust ละเอามา split ทำเป็น array ขนาด 3 ช่อง

    // ถ้าจะเทสการรับ message ให้เปิดตรงนี้
    // document.getElementById("show").innerHTML = message.payloadString;
    var data = message.payloadString.split(",");
    // var data = [];
    // for (var i = 0; i < 3; i++) {
    //     // Generate random number between 0 and 100
    //     var randomNumber = Math.floor(Math.random() * 101);
    //     data.push(randomNumber);
    // }

    document.getElementById("temp").innerHTML = data[0] + " &deg;C";
    document.getElementById("humi").innerHTML = data[1] + " %";
    document.getElementById("dust").innerHTML = data[2] + " mg/m3";

    state = data[3];
    if (state == 1) setStatus("on")
    else if (state == 0) setStatus("off")

    console.log(data);
}

var mqttSend = function (topic,msg){
    var message = new Paho.MQTT.Message(msg);
    message.destinationName = topic;
    client.send(message);
}










