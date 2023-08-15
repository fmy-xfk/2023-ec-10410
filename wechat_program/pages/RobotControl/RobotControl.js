const app = getApp();

var udp = wx.createUDPSocket();
var port = udp.bind();
var sendMsg_flag = false;

Page({
  data: {
    selects: ['舵机1:', '舵机2:', '舵机3:', '舵机4:'],
    robotArmChangeMessage: "_change_position",
    message: '',
  },
  sliderChange: function sliderChange(res) {
    //console.log(this.data.robotArmChangeMessage+res.currentTarget.id+res.detail.value+'_')
    sendMsg_flag = false;
    udp.send({
      address: app.globalData.inputIp,
      port: app.globalData.inputPort,
      message: this.data.robotArmChangeMessage+res.currentTarget.id+res.detail.value+'_',
    });
    /*设置延时等待对端数据过来 */
    setTimeout(function () {
      /*发送按键按下，没有接收到对端发来数据的界面显示*/
      if (!sendMsg_flag) { //延时函数时间内，如果接收到数据会改变sendMsg_flag状态
        wx.showToast({title: '指令发送失败！', icon:'error'});
      }
    }, 300);
  },

  /**
   * 生命周期函数--监听页面加载
   */
  onLoad(options) {
    var that = this;

    udp.onMessage(function (res) {
      var uint8Arr = new Uint8Array(res.message);
      var decodedString=String.fromCharCode.apply(null, uint8Arr);
      //console.log(decodedString);
      that.setData({message: decodedString});
      /*接收到消息在界面上显示*/
      if (decodedString != '') {
        sendMsg_flag = true;
        wx.showToast({title: '指令发送成功！'});
      }
    });
    /*监听数据包消息*/
    udp.onListening(function (res) {
      console.log(res);
    });
  },

  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady() {

  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow() {

  },

  /**
   * 生命周期函数--监听页面隐藏
   */
  onHide() {

  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload() {
    this.unloadPage();
  },

  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh() {

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom() {

  },

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage() {

  }
})