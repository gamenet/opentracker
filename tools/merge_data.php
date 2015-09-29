<?php
$redis = new Redis();
$redis->connect('127.0.0.1', 6300);

if (file_exists('./merge_data.sha')) {
    $evalSha = file_get_contents('./merge_data.sha');
}

if (empty($evalSha)) {
    $evalSha = $redis->script("load", file_get_contents('merge_data.lua'));
    file_put_contents('./merge_data.sha', $evalSha);
}

var_dump($redis->evalSha($evalSha), $redis->getLastError() ?: "No errors");