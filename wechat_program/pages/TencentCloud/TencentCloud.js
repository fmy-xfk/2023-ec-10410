const app = getApp()

var udp = wx.createUDPSocket();
var port = udp.bind();
var num = 0;
var light_flag = false;
var receive_flag = false;
var sendMsg_flag = false;

Page({
  data: {
    workStatus: app.globalData.workStatus,
    workSpeed: app.globalData.workSpeed,
    expressBox: app.globalData.expressBox,
    countdown: 20000,
    hostport: '6655',
    inputIp: app.globalData.inputIp,
    inputPort: app.globalData.inputPort,
    refreshMessage: "_refresh",
  },

  onLoad: function (options) {
    this.initCount();
    var that = this;

    udp.onMessage(function (res) {
      console.log(res.message);
      var uint8Arr = new Uint8Array(res.message);
      /*var encodedString = String.fromCharCode.apply(null, uint8Arr),
        decodedString = decodeURIComponent(escape(encodedString)); //防止中文乱码
      */
      var decodedString=String.fromCharCode.apply(null, uint8Arr);
      //console.log(decodedString);
      that.setData({
        message: decodedString
      })
      app.globalData.expressBox[0] = parseInt(decodedString[0])
      app.globalData.expressBox[1] = parseInt(decodedString[1])
      app.globalData.expressBox[2] = parseInt(decodedString[2])

      /*接收到消息在界面上显示*/
      if (that.data.message != 'ReceiveMsg here') {
        receive_flag = true;
        sendMsg_flag = true;
        wx.showToast({
          title: '数据获取成功！',
        })
      }
    });
    /*监听数据包消息*/
    udp.onListening(function (res) {
      console.log(res);
    });
  },

  initCount() { //初始化倒计时
    let that = this;
    let clock = setInterval(() => {
      if (that.data.countdown === 0) {
        clearInterval(clock); //清除周期
      } else {
        that.data.countdown--
        that.setData({
          countdown: that.data.countdown,
          workStatus: app.globalData.workStatus,
          workSpeed: app.globalData.workSpeed,
          expressBox: app.globalData.expressBox,
        })
        // console.log(that.data.countdown)
      }
    }, 500);
  },
  update() {
    light_flag = true;
    receive_flag = false;
    
    udp.send({
      address: app.globalData.inputIp,
      port: app.globalData.inputPort,
      message: this.data.refreshMessage,
    });
    /*设置延时等待对端数据过来 */
    setTimeout(function () {
      /*发送按键按下，没有接收到对端发来数据的界面显示*/
      if (!receive_flag) { //延时函数时间内，如果接收到数据会改变receive_flag状态
        wx.showToast({
          title: '数据获取失败！',
          icon: 'error'
        })
      }
    }, 200);
    
  },
  /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {

  },
  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {

  },
  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh: function () {

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {

  },
})