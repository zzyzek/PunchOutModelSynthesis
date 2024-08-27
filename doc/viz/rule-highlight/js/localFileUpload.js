// 
// Apache 2.0 License. https://web.dev/articles/read-files
//  Kayce Basques
//  Pete LePage
//  Thomas Steiner
// 

var LocalFileUpload = function(file_id, img_id, load_cb, load_cb_data) {
  this.imgId = img_id;
  this.fileSelectorId = file_id;

  this.img = {};
  this.fileSelector = {};

  this.fileList = {};

  this.img = document.getElementById(img_id);
  this.fileSelector = document.getElementById(file_id);

  this.fileSelector.addEventListener('change', (event) => {
    this.fileList = event.target.files;
    this.readImage(this.fileList[0]);
  });

  this.load_cb = load_cb;
  this.load_cb_data = load_cb_data;

  return this;
}

LocalFileUpload.prototype.setImageById = function(img_id) {
  this.img = document.getElementById(img_id);
}

LocalFileUpload.prototype.setFileSelectorById = function(file_id) {
  this.fileSelector = document.getElementById(file_id);
}

LocalFileUpload.prototype.init = function() {
  //this.img = document.getElementById('ui_exemplarImage');

  //const fileSelector = document.getElementById('ui_fileButton');
  this.fileSelector.addEventListener('change', (event) => {
    this.fileList = event.target.files;
    //console.log(g_fileList);
    this.readImage(this.fileList[0]);
  });


}

LocalFileUpload.prototype.getMetadataForFileList = function(fileList) {
  fileList = ((typeof fileList === "undefined") ? this.fileList : fileList);
  for (const file of fileList) {
    // Not supported in Safari for iOS.
    const name = file.name ? file.name : 'NOT SUPPORTED';
    // Not supported in Firefox for Android or Opera for Android.
    const type = file.type ? file.type : 'NOT SUPPORTED';
    // Unknown cross-browser support.
    const size = file.size ? file.size : 'NOT SUPPORTED';
    console.log("meta:", {file, name, type, size});
  }
}

LocalFileUpload.prototype.readImage = function(file) {
  if (typeof file === "undefined") { return; }
  if (file.type && !file.type.startsWith('image/')) {
    console.log('File is not an image.', file.type, file);
    return;
  }

  const reader = new FileReader();
  reader.addEventListener('load', (event) => {
    console.log("load:", event);
    if (typeof this.img.src !== "undefined") {
      this.img.src = event.target.result;
    }

    if (typeof this.load_cb === "function") {

      //DEBUG
      console.log("calling cb (readImage->load)");

      this.load_cb(this.load_cb_data);
    }
  });
  reader.readAsDataURL(file);
}

LocalFileUpload.prototype.readFile = function(file) {
  const reader = new FileReader();
  reader.addEventListener('load', (event) => {
    const result = event.target.result;
    // Do something with result
  });

  reader.addEventListener('progress', (event) => {
    if (event.loaded && event.total) {
      const percent = (event.loaded / event.total) * 100;
      console.log("Progress:", Math.round(percent));
    }
  });
  reader.readAsDataURL(file);
}

