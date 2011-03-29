<?php
require_once("libraries/ead_parser.inc");

$s = "1973/1982";

if (preg_match('/(\d{4})\D+(\d{4})/', $s, $matches)){
    print_r($matches);
}