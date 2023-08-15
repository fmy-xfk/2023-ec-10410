const musr=require('./users');
const MAX_CNT=10;
var all_data = new Array(MAX_CNT);

exports.set = function(id,data){
  if(id<0 || id>=MAX_CNT) {
    return {"success":false,"err":"INVALID_UID"};
  }
  all_data[id]=data;
  return {"success":true,"err":""};
}

exports.get = function(id){
  if(id<0 || id>=MAX_CNT) {
    return {"success":false,"err":"INVALID_UID"};
  }
  return {"success":true,"data":all_data[id]};
}

exports.clear = function(id){
  all_data[0]=0;all_data[1]=0;all_data[2]=0;
  return {"success":true,"err":""};
}

exports.query = function(ids){
  let cnt=ids.length;
  let ret=[];
  for(let i=0;i<cnt;i++){
    let id=Number(ids[i]);
    if(id<0 || id>65535) {
      return {"success":false,"err":`INVALID_UID:${id}`};
    }
    ret.push(unpacked_data[id]);
  }
  return {"success":true,"data":ret};
}