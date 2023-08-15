var CurrentTimerState = true, ResumeWhenVisible = true;
var int;
function TimerOn(){/*定时更新，间隔1000ms*/
    TimerOff();
    int=self.setInterval("update()",1000);
    CurrentTimerState = true;
}
function TimerOff(){
    int=window.clearInterval(int);
    CurrentTimerState = false;
}
TimerOn();
update();
document.addEventListener("visibilitychange", function(){
    if(document.visibilityState==='hidden'){
        ResumeWhenVisible = CurrentTimerState;
        TimerOff();
    }else{
        if(ResumeWhenVisible === true) TimerOn();
    }
}, false);
function update(){
    $.ajax({type: "GET", dataType: "json", url: "/api/query",success:function(result){
        if(result.success) {
            let data=result.data;
            if(data.connected){
                $("#connected").html("已连接");
            }else{
                $("#connected").html("未连接");
            }
            $("#itm0").html(data.res[0].toString()+"个");
            $("#itm1").html(data.res[1].toString()+"个");
            $("#itm2").html(data.res[2].toString()+"个");
        }else{
            if(result.err=="no_client"){
                msg="发送失败：流水线尚未连接至服务器！";
            }else{
                msg="发送失败！";
            }
            ShowAlert(msg,'danger');
        }
    },error:function(result){
        ShowAlert("查询失败！",'danger');
    }});
}
function send(data){
    data="0"+data;
    $.ajax({type: "GET", dataType: "json", url: "/api/send?mdata="+data,success:function(result){
        if(result.success) {
            ShowAlert("发送成功！",'success');
        }else{
            if(result.err=="no_client"){
                msg="发送失败：流水线尚未连接至服务器！";
            }else{
                msg="发送失败！";
            }
            ShowAlert(msg,'danger');
        }
    },error:function(result){
        ShowAlert("操作失败！",'danger');
    }});
}
function clear(){
    send("N000");
    $.ajax({type: "GET", dataType: "json", url: "/api/clear",success:function(result){
        if(result.success) {
            ShowAlert("复位成功！",'success');
            update();
        }else{
            ShowAlert("复位失败！",'danger');
        }
    },error:function(result){
        ShowAlert("操作失败！",'danger');
    }});
}

function wrapget(filter){
    let content=Number($(filter).val());
    if(content<0) content=0;
    if(content>100) content=100;
    content=content.toFixed();
    while(content.length<3) content="0"+content;
    return content;
}
function setpose0(){
    send("U"+wrapget("#down0"));
    send("V"+wrapget("#down1"));
    send("W"+wrapget("#down2"));
}
function setpose1(){
    send("X"+wrapget("#up0"));
    send("Y"+wrapget("#up1"));
    send("Z"+wrapget("#up2"));
}