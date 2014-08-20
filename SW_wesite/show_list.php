<!--
 stockie show list.php
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
  <title>List display</title>
  <meta http-equiv="refresh" content="1">
 </head>
 
 <script>
 function list_item_check(user_name, secret_key, item_name){
   if (window.XMLHttpRequest) {
    // code for IE7+, Firefox, Chrome, Opera, Safari
    xmlhttp=new XMLHttpRequest();
  } else { // code for IE6, IE5
    xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
  }
  xmlhttp.open("GET","item_check.php?username="+user_name+"&key="+secret_key+"&name="+item_name,true);
  xmlhttp.send();
 }
 
  function list_item_uncheck(user_name, secret_key, item_name){
   if (window.XMLHttpRequest) {
    // code for IE7+, Firefox, Chrome, Opera, Safari
    xmlhttp=new XMLHttpRequest();
  } else { // code for IE6, IE5
    xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
  }
   xmlhttp.open("GET","item_uncheck.php?username="+user_name+"&key="+secret_key+"&name="+item_name,true);
  xmlhttp.send();
 }
 
 </script>
 
 
 <body>
 <p style="font-size:30px">
 
 
<?php

$database_user = "my_secret_database_user";
$database_pass = "my_secret_database_pass";
$database_table = "my_secret_database_table";

//$hw_adr_req= $_REQUEST['hw_adr'];
//$key= $_REQUEST['key'];
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
	$result = mysqli_query($con,"SELECT * FROM " . $user_name . " WHERE value=0");
	$num_rows = mysqli_num_rows($result);
	while($row = mysqli_fetch_array($result)) {
	  echo "<label><input name='" . $row['name'] ."'  type='checkbox' onclick='list_item_check(&quot;" . $user_name . "&quot;,&quot;" . $key . "&quot;,&quot;" . $row['name'] . "&quot;);' >" . $row['name'] . "		</label>";
	  echo "<br>";
	}
	$result = mysqli_query($con,"SELECT * FROM " . $user_name . " WHERE value=1");
	$num_rows = mysqli_num_rows($result);
	while($row = mysqli_fetch_array($result)) {
 	 echo "<label><input name='" . $row['name'] ."'  type='checkbox' onclick='list_item_uncheck(&quot;" . $user_name . "&quot;,&quot;" . $key . "&quot;,&quot;" . $row['name'] . "&quot;);' checked>" . $row['name'] . "</label>";  
  	echo "<br>";
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