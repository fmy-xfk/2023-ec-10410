var fs = require('fs');
var md5 = require('md5-node');

var usrdir=__dirname+"/userlist.json";
var users = JSON.parse(fs.readFileSync(usrdir)).users;
var online_users = {};
var m_sessions = {};

function randomString(len) {
  len = len || 32;
  var $chars = 'ABCDEFGHJKMNPQRSTWXYZabcdefhijkmnprstwxyz2345678';    /****默认去掉了容易混淆的字符oO，Ll,9gq,Vv,Uu,I1****/
  var maxPos = $chars.length;
  var pwd = '';
  for (i = 0; i < len; i++) {
    pwd += $chars.charAt(Math.floor(Math.random() * maxPos));
  }
  return pwd;
}

function save_users(){
  fs.writeFileSync(usrdir,JSON.stringify({"users":users}));
}

exports.login = function(name,pwd){
  var usr=users.find(e=>e.name===name);
  if(usr!=undefined && usr.pwd_md5===md5(pwd)){
    let sID = randomString(32);
    online_users[name]={"time":new Date().getTime(),"sessionID":sID};
    m_sessions[sID]=name;
    return online_users[name]['time'];
  }
  return -1;
}

exports.logout = function(name) {
  if(online_users[name]!=undefined){
    delete m_sessions[online_users[name]["sessionID"]];
    delete online_users[name];
    return true;
  }else{
    return false;
  }
}

exports.chpwd = function(name,oldpwd,newpwd) {
  var usr=users.find(e=>e.name===name);
  if(usr!=undefined && usr.pwd_md5===md5(oldpwd)){
    //console.log(usr,newpwd);
    usr.pwd_md5=md5(newpwd);
    save_users();
    return true;
  }
  return false;
}

function checkValidity(name,timestamp){
  if(online_users[name]){
    return online_users[name]['time']===timestamp;
  }else{
    return false;
  }  
}

exports.checkSessionID = function (sID){
  if(m_sessions[sID]!=undefined){
    return true;
  }else{
    return false;
  }
}

exports.isAdmin = function(req) {
  if(req.session === undefined) return false;
  if(req.session.user === 'admin'){
    return checkValidity(req.session.user,req.session.timestamp);
  }else{
    return false;
  }
}

exports.isLogined = function(req) {
  if (req === undefined) {
    console.log("req参数未设置！");
    return false;
  }
  if (req.session === undefined) return false;
  if (req.session.user===undefined || req.session.user===""){
    return false;
  }else{
    return checkValidity(req.session.user,req.session.timestamp);
  }
};

exports.getSessionID = function(req) {
  if(exports.isLogined(req)){
    return online_users[req.session.user]['sessionID'];
  }else{
    return null;
  }
}

exports.getUser = function(req){
  if(req.session === undefined) return undefined;
  if (req.session.user!=undefined && req.session.user!="" && checkValidity(req.session.user,req.session.timestamp)){
    return req.session.user;
  }else{
    return undefined;
  }
}