const app = getApp();

var udp = wx.createUDPSocket();
var port = udp.bind();
var num = 0;
var light_flag = false;
var receive_flag = false;
var sendMsg_flag = false;

Page({
  data: {
    sendMsgbuff: '',
    message: '',
    selects: [{
      name: '舵机1控制:',
      clickId: -1
    }, {
      name: '舵机2控制:',
      clickId: -1
    }, {
      name: '舵机3控制:',
      clickId: -1
    }],
    lightOnMsg: '_light_on',
    lightOffMsg: '_light_off',
    unloadMsg: 'UnoladPage',
    num_speed: 0,
    speedChangeMsg: '_change_speed', 
  },

  changeOil: function (e) {
    this.setData({
      num_speed: e.target.dataset.num
    })
    if(this.data.num_speed == 0){
      app.globalData.workStatus = "关闭";
      app.globalData.workSpeed = 0;
    }
    else{
      app.globalData.workStatus = "开启"
      app.globalData.workSpeed = 30 * this.data.num_speed;
    }
    console.log(this.data.speedChangeMsg+e.target.dataset.num);

    light_flag = true;
    receive_flag = false;
    if (this.JudgeDataValidity()) {
      udp.send({
        address: app.globalData.inputIp,
        port: app.globalData.inputPort,
        message: this.data.speedChangeMsg+e.target.dataset.num,
      });
      /*设置延时等待对端数据过来 */
      setTimeout(function () {
        /*发送按键按下，没有接收到对端发来数据的界面显示*/
        if (!receive_flag) { //延时函数时间内，如果接收到数据会改变receive_flag状态
          wx.showToast({
            title: '速度控制失败！',
            icon: 'error'
          })
        }
      }, 200);
    }
  },

  changeColor: function (res) {
    let choseChange = "selects[" + res.currentTarget.id + "].clickId"
    if (this.data.selects[res.currentTarget.id].clickId == (+res.currentTarget.id) + 1) {
      this.setData({
        [choseChange]: -((+res.currentTarget.id) + 1)
      })
      return;
    } else {
      this.setData({
        [choseChange]: (+res.currentTarget.id) + 1
      })
    }
  },
  /**
   * 输入要发送的信息
   */
  addMessage(res) {
    this.data.sendMsgbuff = res.detail.value
  },
  /*休眠函数*/
  sleep: function (milSec) {
    return new Promise(resolve => {
      setTimeout(resolve, milSec)
    })
  },

  onLoad() {
    var that = this;

    udp.onMessage(function (res) {
      var uint8Arr = new Uint8Array(res.message);
      var decodedString=String.fromCharCode.apply(null, uint8Arr);

      that.setData({message: decodedString});
      /*接收到消息在界面上显示*/
      if (that.data.message != 'ReceiveMsg here') {
        receive_flag = true;
        sendMsg_flag = true;
        wx.showToast({
          title: '发送消息成功！',
        })
        // app.globalData.expressBox[0] = app.globalData.expressBox[0] + 1;
        // console.log(app.globalData.expressBox[0]);
      }
    });
    /*监听数据包消息*/
    udp.onListening(function (res) {
      console.log(res);
    });
  },

  //判断输入数据的有效性
  JudgeDataValidity: function () {
    if (!app.globalData.inputIp) {
      wx.showModal({
        title: '提示',
        content: '请输入要连接设备的IP',
      })
      return false
    } else if (!app.globalData.inputPort) {
      wx.showModal({
        title: '提示',
        content: '请输入要连接设备的端口号',
      })
      return false
    } else if (!app.globalData.inputIp && !app.globalData.inputPort) {
      wx.showModal({
        title: '提示',
        content: '请输入要连接设备的IP和端口号',
      })
      return false
    } else if (!this.data.sendMsgbuff && !light_flag) {
      wx.showModal({
        title: '提示',
        content: '请输入要发送到设备的消息',
      })
      return false
    } else {
      return true
    }
  },

  SendMsg: function SendMsg() {
    light_flag = false;
    receive_flag = false;
    if (this.JudgeDataValidity()) {
      udp.send({
        address: app.globalData.inputIp,
        port: app.globalData.inputPort,
        message: this.data.sendMsgbuff,
      });
      /*设置延时等待对端数据过来 */
      setTimeout(function () {
        /*发送按键按下，没有接收到对端发来数据的界面显示*/
        if (!receive_flag) { //延时函数时间内，如果接收到数据会改变receive_flag状态
          wx.showToast({
            title: '发送消息失败！',
            icon: 'error'
          })
        }
      }, 200);
    }
  },
  light_off: function light_off(res) {
    // console.log(res.currentTarget.id)
    light_flag = true;
    receive_flag = false;
    if (this.JudgeDataValidity()) {
      udp.send({
        address: app.globalData.inputIp,
        port: app.globalData.inputPort,
        message: this.data.lightOnMsg + res.currentTarget.id,
      });
      /*设置延时等待对端数据过来 */
      setTimeout(function () {
        /*发送按键按下，没有接收到对端发来数据的界面显示*/
        if (!receive_flag) { //延时函数时间内，如果接收到数据会改变receive_flag状态
          wx.showToast({
            title: '舵机弹出失败！',
            icon: 'error'
          })
        }
      }, 200);
    }
  },
  light_on: function light_on(res) {
    // console.log(res.currentTarget.id)
    light_flag = true;
    receive_flag = false;
    if (this.JudgeDataValidity()) {
      udp.send({
        address: app.globalData.inputIp,
        port: app.globalData.inputPort,
        message: this.data.lightOffMsg + res.currentTarget.id,
      });
      /*设置延时等待对端数据过来 */
      setTimeout(function () {
        /*发送按键按下，没有接收到对端发来数据的界面显示*/
        if (!receive_flag) { //延时函数时间内，如果接收到数据会改变receive_flag状态
          wx.showToast({
            title: '舵机收回失败！',
            icon: 'error'
          })
        }
      }, 200);
    }
  },
  unloadPage: function () {
    udp.send({
      address: app.globalData.inputIp,
      port: app.globalData.inputPort,
      message: this.data.lightOffMsg,
    });
  },
  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady: function () {

  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {
  },

  onUnload: function () {
    this.unloadPage();
    // udp.close();//退出页面时将socket关闭
  }
})