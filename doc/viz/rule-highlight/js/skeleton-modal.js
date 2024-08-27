/*
  The MIT License (MIT)

  Copyright (c) 2023 Paul McClean (https://codepen.io/paulmcclean/pen/YGXWQY)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

function skeleton_modal_init() {

  var modal = document.querySelector('.modal');
  if ((typeof modal === "undefined") ||
      (modal == null) ||
      (modal.length==0)) { return; }

  var closeButtons = document.querySelectorAll('.close-modal');
  if ((typeof closeButtons === "undefined") ||
      (closeButtons.length==0)) { return; }

  // set open modal behaviour
  document.querySelector('.open-modal').addEventListener('click', function() {
    modal.classList.toggle('modal-open');
  });
  // set close modal behaviour
  for (i = 0; i < closeButtons.length; ++i) {
    closeButtons[i].addEventListener('click', function() {
      modal.classList.toggle('modal-open');
    });
  }
  // close modal if clicked outside content area
  document.querySelector('.modal-inner').addEventListener('click', function() {
    modal.classList.toggle('modal-open');
  });
  // prevent modal inner from closing parent when clicked
  document.querySelector('.modal-content').addEventListener('click', function(e) {
    e.stopPropagation();
  });

}
