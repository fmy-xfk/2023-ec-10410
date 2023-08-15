var createError = require('http-errors');
var express = require('express');
var path = require('path');
var cookieParser = require('cookie-parser');
var logger = require('morgan');
var ejs = require('ejs');
var session = require('express-session');

var musr = require('./routes/users');
var indexRouter = require('./routes/index');
var authRouter = require('./routes/auth');
var apiRouter = require('./routes/api');

var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.engine('ejs', ejs.__express);
app.set('view engine', 'ejs');

//session for login(must before routers)
app.use(session({
  name: "datang.userdata",
  secret: "wind-turbines-datang",//randomString(64),
  resave: false,
  saveUninitialized: false,
  cookie: {maxAge: 1*24*60*60*1000}//1 day
}));

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use('/', indexRouter);
app.use('/auth', authRouter);
app.use('/api', apiRouter);

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  next(createError(404));
});

// error handler
app.use(function(err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};
  if(err.status!=403 && err.status!=404){
    console.log(err);
  }
  // render the error page
  res.status(err.status || 500);
  res.render('error', {"status" : err.status, "user":musr.getUser(req), "err":err});
});

module.exports = app;
