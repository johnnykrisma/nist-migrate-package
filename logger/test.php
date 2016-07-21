<?php
include 'MigrateLogger.php';


$logger = new MigrateLogger;

$logger->logMessage('I am the Fooo!', 'foolog.log');

var_dump($logger);



