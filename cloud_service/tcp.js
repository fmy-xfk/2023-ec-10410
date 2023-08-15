const net = require('net');
const dataman = require('./routes/dataman');
const { time } = require('console');

//统计连接客户端的个数
var count = 0, g_sock=undefined, g_refresh_time=0;

// 创建TCP服务器
const tcpserver = new net.createServer();

function is_connected(){
    if(g_sock!=undefined && (new Date().getTime()-g_refresh_time)<30*1000) return true;
    return false;
};

tcpserver.is_connected=is_connected;

tcpserver.send=function(data){
    if(is_connected()){
        g_sock.write(data);
        return {"success":true,"err":""};
    }else{
        g_sock=undefined;
        return {"success":false,"err":"no_client"};
    }
};

tcpserver.setEncoding = 'UTF-8';
//获得一个连接，该链接自动关联scoket对象
tcpserver.on('connection', sock => {
    sock.name = `${sock.remoteAddress}:${sock.remotePort}`
    g_sock=sock;g_refresh_time=new Date().getTime();
    console.log(`当前连接客户端数：${++count}`);
    // 为这个socket实例添加一个"data"事件处理函数
    //客户端发送数据时触发data事件
    //接收client发来的信息  
    sock.on('data', data => {
        // 回发该数据，客户端将收到来自服务端的数据
        try{
            if(data[0]==64){//@
                g_refresh_time=new Date().getTime();
            }else if(data[0]==67){//C
                console.log(`[C_CMD]from=${sock.name},data="${data}"`);
                let raws=String(data).split("\n");
                raws.forEach(elem => {
                    let id=Number(elem.substring(1,3)),dat=Number(elem.substring(3));
                    dataman.set(id,dat);
                });
            }else{
                console.log(`[BAD_CMD]from=${sock.name},data="${data}"`);
            }
        }catch{
            console.log("Error: Bad reception!");
        }
        //sock.write(`服务端收到了你发送的一条信息：${data}`);
    });
    //为socket添加error事件处理函数
    sock.on('error', error => { //监听客户端异常
        console.log('error' + error);
        g_sock=undefined;
        sock.destroy();
        //sock.end();
    });

    // 为这个socket实例添加一个"close"事件处理函数
    //当对方的连接断开以后的事件
    //服务器关闭时触发，如果存在连接，这个事件不会被触发，直到所有的连接关闭
    sock.on('close', () => {
        g_sock=undefined;
        console.log(`客户端${sock.name}下线了`);
        --count;
    });
})

module.exports = tcpserver;

