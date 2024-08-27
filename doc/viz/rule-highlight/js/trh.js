// LICENSE: CC0
//

// YIKES!!!
// https://stackoverflow.com/questions/23497925/how-can-i-stop-the-alpha-premultiplication-with-canvas-imagedata/23501676#23501676
//
// canvas putImageData does not guarantee getImageData is
// the same.
//
// In chrome, it looks like png data is respcted. FF tramples
// on the data (alpha 255, so who knows what's going on), littering
// the source image with pixels that screw up the tiling procedure.
//
// To work around this, 'flatten' pixel values to nearest
// and most frequent pixel with L1 norm distance l1_dist.
//
function flatten_img_data(raw_data, l1_dist) {
  let map_count= {};
  let tot_count = 0;
  let n = raw_data.length;
  let cluster_rep = [];
  let cluster_idx = {};
  let pxl_count = {};
  let cluster_count=0;

  let pool = [];

  for (let ii=0; ii<n; ii+=4) {
    let s =
      raw_data[ii+0].toString() + ":" +
      raw_data[ii+1].toString() + ":" +
      raw_data[ii+2].toString() + ":" +
      raw_data[ii+3].toString();

    if (s in map_count) {
      map_count[s]++;
      continue;
    }
    map_count[s] = 1;
  }

  let pxl = [ 0, 0, 0, 0 ];
  for (let ii=0; ii<n; ii+=4) {
    let s =
      raw_data[ii+0].toString() + ":" +
      raw_data[ii+1].toString() + ":" +
      raw_data[ii+2].toString() + ":" +
      raw_data[ii+3].toString();

    pxl[0] = raw_data[ii+0];
    pxl[1] = raw_data[ii+1];
    pxl[2] = raw_data[ii+2];
    pxl[3] = raw_data[ii+3];

    let rep_idx = -1;
    for (let cidx=0; cidx<cluster_rep.length; cidx++) {
      let d = Math.abs(pxl[0] - cluster_rep[cidx][0]) +
        Math.abs(pxl[1] - cluster_rep[cidx][1]) +
        Math.abs(pxl[2] - cluster_rep[cidx][2]) +
        Math.abs(pxl[3] - cluster_rep[cidx][3]);

      if (d < l1_dist) {
        rep_idx = cidx;
      }
    }

    // add a new cluster
    //
    if (rep_idx < 0) {
      cluster_rep.push([ pxl[0], pxl[1], pxl[2], pxl[3] ]);
      cluster_idx[s] = cluster_count;
      cluster_count++;
    }
    else {
      cluster_idx[s] = rep_idx;
    }

  }

  //debug
  /*
  console.log("cluster_count:", cluster_count);
  console.log("before, cluster_rep:");
  for (let ii=0; ii<cluster_rep.length; ii++) {
    console.log(ii, cluster_rep[ii]);
  }
  */

  // pick maximum frequency representative
  //
  for (let ii=0; ii<n; ii+=4) {
    let s =
      raw_data[ii+0].toString() + ":" +
      raw_data[ii+1].toString() + ":" +
      raw_data[ii+2].toString() + ":" +
      raw_data[ii+3].toString();

    let cidx = cluster_idx[s];
    let _rep = cluster_rep[cidx];

    let _rep_count = map_count[_rep];
    let _cur_count = map_count[s];

    if (_cur_count > _rep_count) {
      cluster_rep[cidx] = s;
    }

  }

  /*
  console.log("before, cluster_rep:");
  for (let ii=0; ii<cluster_rep.length; ii++) {
    let s =
      cluster_rep[ii][0].toString() + ":" +
      cluster_rep[ii][1].toString() + ":" +
      cluster_rep[ii][2].toString() + ":" +
      cluster_rep[ii][3].toString() ;

    console.log(ii, s, cluster_rep[ii], map_count[s]);
  }
  */

  for (let ii=0; ii<n; ii+=4) {
    let s =
      raw_data[ii+0].toString() + ":" +
      raw_data[ii+1].toString() + ":" +
      raw_data[ii+2].toString() + ":" +
      raw_data[ii+3].toString();
    let cidx = cluster_idx[s];
    let pxl = cluster_rep[cidx];

    raw_data[ii+0] = pxl[0];
    raw_data[ii+1] = pxl[1];
    raw_data[ii+2] = pxl[2];
  }

}

function color_count(raw_data) {
  let map_count= {};
  let map_occur = {};
  let c_count = 0;
  let n = raw_data.length;

  for (let ii=0; ii<n; ii+=4) {
    let s =
      raw_data[ii+0].toString() + ":" +
      raw_data[ii+1].toString() + ":" +
      raw_data[ii+2].toString() + ":" +
      raw_data[ii+3].toString();

    if (s in map_count) {
      map_count[s]++;
      continue;
    }
    map_count[s] = 1;
    map_occur[s] = ii;

    c_count++;
  }


  return { "tot_count": c_count, "occur":map_occur, "count": map_count };
}

function imgPNG(w,h) {
  let canvas = document.createElement('canvas');
  canvas.height = h;
  canvas.width = w;
  let ctx = canvas.getContext('2d', {"alpha":false});

  ctx.webkitImageSmoothingEnabled = false;
  ctx.imageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.oImageSmoothingEnabled = false;

  return ctx.getImageData(0, 0, canvas.width, canvas.height);
}

function b64PNG(w,h,raw_data) {
  let canvas = document.createElement('canvas');
  canvas.height = h;
  canvas.width = w;
  let ctx = canvas.getContext('2d', {"alpha":false});

  ctx.webkitImageSmoothingEnabled = false;
  ctx.imageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.oImageSmoothingEnabled = false;

  let im = ctx.getImageData(0, 0, canvas.width, canvas.height);
  im.data.set( raw_data );
  ctx.putImageData(im, 0, 0);
  return canvas.toDataURL();
}

function imgData(ui_id) {
  let canvas = document.createElement('canvas');
  let ctx = canvas.getContext('2d', {"alpha":false});

  ctx.webkitImageSmoothingEnabled = false;
  ctx.imageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.oImageSmoothingEnabled = false;


  let img = document.getElementById(ui_id);
  canvas.height = img.naturalHeight;
  canvas.width = img.naturalWidth;

  console.log("imgData: ui_id:", ui_id, canvas.width, canvas.height);

  ctx.drawImage(img, 0, 0, img.naturalWidth, img.naturalHeight);
  let base64String = canvas.toDataURL();


  let _raw = ctx.getImageData(0, 0, canvas.width, canvas.height);
  let _b64 = base64String;

  //console.log(g_raw);
  //console.log(base64String);
  return _raw;
}

function imgDatab64(ui_id) {
  let canvas = document.createElement('canvas');
  let ctx = canvas.getContext('2d', {"alpha":false});

  ctx.webkitImageSmoothingEnabled = false;
  ctx.imageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.oImageSmoothingEnabled = false;

  let img = document.getElementById(ui_id);
  canvas.height = img.naturalHeight;
  canvas.width = img.naturalWidth;

  ctx.drawImage(img, 0, 0, img.naturalWidth, img.naturalHeight);
  let base64String = canvas.toDataURL();


  let _raw = ctx.getImageData(0, 0, canvas.width, canvas.height);
  let _b64 = base64String;

  return _b64;
}

function imgDataLoad(imgdata, ui_id) {
  let canvas = document.createElement("canvas");
  let ctx = canvas.getContext("2d", {"alpha":false});

  ctx.webkitImageSmoothingEnabled = false;
  ctx.imageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.oImageSmoothingEnabled = false;

  canvas.width = imgdata.width;
  canvas.height = imgdata.height;

  ctx.fillStyle = "red";
  ctx.rect(0,0,canvas.width,canvas.height);
  ctx.fill();

  ctx.putImageData(imgdata, 0, 0);

  let img = ((typeof ui_id === "undefined") ? new Image() : document.getElementById(ui_id));
  img.src = canvas.toDataURL();
  return img;
}

function deactivateShimmer(ui_id) {
  let ele = document.getElementById(ui_id);
  if (ele == null) { return; }
  ele.style.animation = "none";
}

//---------
//---------
//---------
//---------

// checkbox handling

function uiULCheckbox(base_id) {

  let ele_x = document.getElementById( base_id + "_x" );
  let ele_y = document.getElementById( base_id + "_y" );
  let ele_locked = document.getElementById( base_id + "_locked" );

  if (ele_locked.checked) {
    ele_y.style["color"] = "rgba(0,0,0,0.5)";
    ele_y.style["background-color"] = "rgba(0,0,0,0.125)";
    ele_y.disabled = true;
    ele_y.value = ele_x.value;
  }
  else {
    ele_y.style["color"] = "rgba(0,0,0,1.0)";
    ele_y.style["background-color"] = "rgba(1,1,1,0.0)";
    ele_y.disabled = false;

  }

}

function uiULVal(base_id) {

  let ele_x = document.getElementById( base_id + "_x" );
  let ele_y = document.getElementById( base_id + "_y" );
  let ele_locked = document.getElementById( base_id + "_locked" );

  if (ele_locked.checked) {
    ele_y.value = ele_x.value;
  }

}

