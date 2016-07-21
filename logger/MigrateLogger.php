<?php
/**
 * I am a simple logging utility for the NIST migrate scripts.
 * Created by PhpStorm.
 * User: jfa
 * Date: 8/21/15
 * Time: 11:11 AM
 */

class MigrateLogger {

  /**
   * I am the name of the directory where log files are kept.
   * @var string
   */
  private $logDirectoryName = 'log';

  /**
   * I am the computed directory where the lod directory will live.
   * @var string
   */
  private $logDirectory = '';


  /**
   * I am the constructor.
   */
  public function MigrateLogger(){

    $this->setLogDirectory();
  }

  /**
   * I log a message to a file. If not passed $message or $file I return false
   * else I log the message and return true.
   * @param string $message - I am the message to log.
   * @param string $file - I am the file to log the message to.
   * @return bool - I return true if I logged something, false if I did not.
   */
  public function logMessage( $message = '', $file = '' ){

    if( !strlen( $message ) or !strlen( $file )){
      return false;
    }

    $date = new DateTime();
    $formattedDate = $date->format('Y-m-d H:i:s');
    $logMessage = $formattedDate . ' ' . $message . PHP_EOL;

    //drush_log($this->getLogPath( $file ), $type = 'warning', $error = null);

    file_put_contents( $this->getLogPath( $file ), $logMessage, FILE_APPEND );

    return true;
  }

  /**
   * I return the system appended with the log file name passed to me
   * @param $logFile
   * @return string
   */
  private function getLogPath( $logFile ){

    return $this->logDirectory . $logFile;
  }

  /**
   * I set the system path to the directory where log files are kept
   */
  private function setLogDirectory(){

    $this->logDirectory = realpath(dirname(__FILE__)). '/' . $this->logDirectoryName . '/';
  }
}
