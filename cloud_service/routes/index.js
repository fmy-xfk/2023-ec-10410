var express = require('express');
var musr = require('./users');
var fs = require('fs');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  if(musr.isLogined(req)){
    res.render('index',{"user":musr.getUser(req)});
  }else{
    res.render('index2',{"user":musr.getUser(req)});
  }
});

module.exports = router;
