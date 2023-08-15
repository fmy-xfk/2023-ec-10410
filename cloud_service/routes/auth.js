var express = require('express');
var createError = require('http-errors');
var session = require('express-session');
var musr = require('./users');
var fs = require('fs');
var router = express.Router();

router.get('/', function(req, res, next) {
    if(musr.isLogined(req)){
        res.render('auth/index',{"user":musr.getUser(req)});
    }else{
        next(createError(403));
    }
});

router.get('/login', function(req, res, next) {
    res.redirect('/auth');
});

router.post('/logout', function(req, res, next) {
    var cur_user=musr.getUser(req);
    if(cur_user) {
        musr.logout(cur_user);
        req.session.destroy();
        res.send({"success":true,"err":""});
    }else{
        res.send({"success":false,"err":"not logined"});
    }
});

router.post('/login', function(req,res,next){
    if(musr.isLogined(req)) {
        res.send({"success":false,"err":"already logined"});
    }else{
        var ret = musr.login(req.body.username,req.body.password);
        if(ret===-1){
            req.session.destroy();
            res.send({"success":false,"err":"bad user or pwd"});
        }else{
            req.session.user = req.body.username;
            req.session.timestamp = ret;
            res.send({"success":true,"err":""});
        }
    }
});

router.post('/chpwd',function(req,res,next){
    if(!musr.isLogined(req)) {
        res.send({"success":false,"err":"not logined"});
    }else{
        res.send({"success":musr.chpwd(req.session.user,req.body.old,req.body.new),"err":""});
    }
});

module.exports = router;