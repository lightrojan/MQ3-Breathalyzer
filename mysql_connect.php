<?php

$servername = "xx.xxx.xxx.xxx";
$username = "xxx";
$password = "xxx";
$dbname = "xxx";
$temp1=$_POST["yourdata"];

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "INSERT INTO xxxxx (yourdata)
VALUES ('$temp1')";

if ($conn->query($sql) === TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
?>
