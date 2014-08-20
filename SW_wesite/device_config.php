<!--
 stockie device config.php
 copyright (c) Bogdan Raducanu, 2014
 Created: August 2014
  Author: Bogdan Raducanu
  bogdan@electrobob.com

	www.electrobob.com 
	

Released under GPLv3.
Please refer to LICENSE file for licensing information.	
-->
<html>
 <head>
  <title>COnfiguration and status of available devices</title>
	<meta http-equiv="refresh" content="1">
<style>
table,th,td
{
border:1px solid black;
}
</style>
 </head>

 <body>
 <p style="font-size:25px">
<?php

$database_user = "my_secret_database_user";
$database_pass = "my_secret_database_pass";
$database_table = "my_secret_database_table";

$hw_adr_req= $_REQUEST['hw_adr'];
$key= $_REQUEST['key'];
$user_name= $_REQUEST['username'];
$key= $_REQUEST['key'];
// Create connection
$con=mysqli_connect("", $database_user, $database_pass, $database_table);
if (mysqli_connect_errno()) {
  echo "Failed to connect to MySQL: " . mysqli_connect_error();
} else
{
	$result = mysqli_query($con, "SELECT * FROM users WHERE user_name ='". $user_name . "' AND pass='" . $key . "'"); //Check for user
	$num_rows = mysqli_num_rows($result);
	if($num_rows > 0){
	//echo "User ". $user_name ." found<br>";
	echo $user_name . "'s shopping list:<br>";
	$result = mysqli_query($con,"SELECT * FROM " . $user_name);
	$num_rows = mysqli_num_rows($result);
	echo "<table style=\"width:700px\">";
	echo "<tr> <td>ID</td><td>Name</td><td>Value</td><td>Tag updated</td><td>Battery</td><td>List updated</td></tr>";
	while($row = mysqli_fetch_array($result)) {
 	 echo "<tr><td>" . $row['id'] . "</td><td>" . $row['name'] . "</td>";
  	if($row['value'] == "1"){
  	echo "<td>Full</td>";
  	} else 
  	echo "<td bgcolor=\"#FF5555\">EMPTY</td>"; //check page source, why do i need \ to escape here instead of &quot;
    	echo "<td>"  . $row['time_tag'] . "</td><td>"  . $row['batt'] . "</td><td>"  . $row['time_list'] . "</td> </tr>";
	}
	echo "</table>";
}
}
mysqli_close($con);

/*
echo "Bogdan's device list</p>";
if ($key == "secret"){
// Create connection
$con=mysqli_connect("", $database_user, $database_pass, $database_table);

// Check connection
if (mysqli_connect_errno()) {
  echo "Failed to connect to MySQL: " . mysqli_connect_error();
} else
$result = mysqli_query($con,"SELECT * FROM devices");
$num_rows = mysqli_num_rows($result);

echo "<table style=\"width:700px\">";
echo "<tr> <td>ID</td><td>Name</td><td>Value</td><td>Tag updated</td><td>Battery</td><td>List updated</td></tr>";

while($row = mysqli_fetch_array($result)) {
  echo "<tr><td>" . $row['id'] . "</td><td>" . $row['name'] . "</td>";
  if($row['value'] == "1"){
  	echo "<td>Full</td>";
  	} else 
  	echo "<td bgcolor=\"#FF5555\">EMPTY</td>"; //check page source, why do i need \ to escape here instead of &quot;
    echo "<td>"  . $row['time_tag'] . "</td><td>"  . $row['batt'] . "</td><td>"  . $row['time_list'] . "</td> </tr>";

}

echo "</table>";
$result = mysqli_query($con,"SELECT * FROM devices WHERE value=1");
$num_rows = mysqli_num_rows($result);

mysqli_close($con);
}*/




?>
 </body>
</html>