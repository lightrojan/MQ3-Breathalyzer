<?php

$servername = "18.191.104.220";
$username = "root";
$password = "root";
$dbname = "mddbac";
$temp1=$_POST["yourdata"];

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "INSERT INTO breath_readings (yourdata)
VALUES ('$temp1')";

if ($conn->query($sql) === TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
?>
