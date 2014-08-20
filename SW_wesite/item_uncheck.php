<!--
 stockie item uncheck.php
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
  <title>UNCHECK item by NAME</title>
 </head>
 <body>
<?php

$database_user = "my_secret_database_user";
$database_pass = "my_secret_database_pass";
$database_table = "my_secret_database_table";

$item_name_req= $_REQUEST['name'];
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
	$result = mysqli_query($con,"SELECT * FROM " . $user_name . " WHERE name = '" . $item_name_req . "'");
	$num_rows = mysqli_num_rows($result);
	if($num_rows > 0){
	echo "Requested item unchecked<br>";
	$row = mysqli_fetch_array($result); 
	  echo $row['id'] . $row['name'];
	  echo "<br>";
	
	mysqli_query($con,"UPDATE " . $user_name . " SET value=0 WHERE name = '" . $item_name_req . "'");
	mysqli_query($con,"UPDATE " . $user_name . " SET time_list = now()  WHERE name = '" . $item_name_req . "'");
	} 
	else
	{
	echo "<br> error: item not found";
	}
}else 
	{
	echo "User or password not correct<br>";
	}

}
mysqli_close($con);



?>
 </body>
</html>

