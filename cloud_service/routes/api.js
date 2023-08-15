var express = require('express');
const dataman = require('./dataman')
const fs = require('fs');
const musr = require('./users');
var router = express.Router();
var tcpser = require('../tcp');

router.use((req, res, next)=>{
  if(musr.isLogined(req)){
    next();
  }else{
    res.send({"success":false, "err": "NO_LOGIN"});
  }
});

router.post('/update',function(req, res, next){
  var uid=Number(req.body.uid),data=req.body.newdata;
  console.log(uid,data)
  if(req.body.uid===undefined || uid<0 || uid>=10) {
    res.send({"success":false,"err":"INVALID_UID"});
    return;
  }
  if(data===undefined) {
    res.send({"success":false,"err":"UNDEF_DATA"});
    return;
  }
  res.send(dataman.set(uid,data));
});

router.get('/clear',function(req, res, next){
  res.send(dataman.clear());
});

router.get('/send', function(req, res, next) {
  var data=req.query.mdata;
  console.log(data);
  if(data===undefined){
    res.send({"success":false, "err": "INTERNAL_ERR"})
  }else{
    res.send(tcpser.send(data))
  }
});

function myget(id){
  let dat=dataman.get(id)['data'];
  if(dat===undefined || dat===null){
    return "0";
  }else{
    return dat.toString();
  }
}
router.get('/query', function(req, res, next) {
  res.send({"success":true,"data":{
    "connected":tcpser.is_connected(),
    "res":[myget(0),myget(1),myget(2)]
  }});
});
module.exports = router;
