<?php
$to = "fix@fix8.org";
$email = $_REQUEST['email'] ;
$subject = $_REQUEST['subject'] ;
$message = $_REQUEST['message'] ;
$name = $_REQUEST['name'] ;
$headers = "Reply-to: noreply@fix8.org\r\n" . "From: contact@fix8.org\r\n" . "X-Mailer: php";
$body = "From: $name\n\nEmail: $email\n\nSubject:$subject\n\nMessage: $message";
$sent = mail($to, $subject, $body, $headers) ;
if($sent)
	{print "Your feedback was sent successfully."; }
else
	{print "We encountered an error sending your feedback."; }
?>
<br>
<br>
<a href="index.html"><img src="fix8/fix8_Logo_RGB.png" width="88" height="88"/><br><br>Return to Fix8 Home</a>

