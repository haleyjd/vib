drawTest = function () {
 
  var init = function () {
    Video.openWindow(256, 128);
    Video.setCaption("Burn, Suffer, Die!");
    Video.createFramebufferTexture();
    
    gl.Enable(gl.TEXTURE_2D);
    gl.MatrixMode(gl.MODELVIEW);
    gl.LoadIdentity();
    gl.MatrixMode(gl.PROJECTION);
    gl.LoadIdentity();
    gl.Ortho(0.0, 256.0, 128.0, 0.0, -1.0, 1.0);
    gl.Disable(gl.DEPTH_TEST);
    gl.ClearColor(0.0, 0.0, 0.0, 0.0);
    gl.Clear(gl.COLOR_BUFFER_BIT);
  };
  
  var render = function () {
    Video.renderFramebuffer();
  }; 

  var stage;
  var FIRE_WIDTH  = 256;
  var FIRE_HEIGHT = 128;
  var firePal = [];
  var firePixels = [];
  var fireBuffer = [];
  var container = null;
  var color;
  var canvas;
  var bmp;
            
  var fireRGB = [
    0x07,0x07,0x07,0x1F,0x07,0x07,0x2F,0x0F,0x07,0x47,0x0F,0x07,0x57,0x17,0x07,0x67,
    0x1F,0x07,0x77,0x1F,0x07,0x8F,0x27,0x07,0x9F,0x2F,0x07,0xAF,0x3F,0x07,0xBF,0x47,
    0x07,0xC7,0x47,0x07,0xDF,0x4F,0x07,0xDF,0x57,0x07,0xDF,0x57,0x07,0xD7,0x5F,0x07,
    0xD7,0x5F,0x07,0xD7,0x67,0x0F,0xCF,0x6F,0x0F,0xCF,0x77,0x0F,0xCF,0x7F,0x0F,0xCF,
    0x87,0x17,0xC7,0x87,0x17,0xC7,0x8F,0x17,0xC7,0x97,0x1F,0xBF,0x9F,0x1F,0xBF,0x9F,
    0x1F,0xBF,0xA7,0x27,0xBF,0xA7,0x27,0xBF,0xAF,0x2F,0xB7,0xAF,0x2F,0xB7,0xB7,0x2F,
    0xB7,0xB7,0x37,0xCF,0xCF,0x6F,0xDF,0xDF,0x9F,0xEF,0xEF,0xC7,0xFF,0xFF,0xFF
  ];
  
  var firePixelsToFireBuffer = function (i) {
    var p = firePixels[i];
    var c = firePal[p];
    fireBuffer[i] = c.b | (c.g << 8) | (c.r << 16) | (0xff << 24);
  }

  var start = function () {
    for(var i = 0; i < 37; i++) {
      firePal[i] = {
         r : fireRGB[i * 3 + 0], //16 * i,
         g : fireRGB[i * 3 + 1], //16 * i,
         b : fireRGB[i * 3 + 2] //16 * i
      }
    }
               
    for(var i = 0; i < FIRE_WIDTH*FIRE_HEIGHT; i++) {
      firePixels[i] = 0;
      firePixelsToFireBuffer(i);
    }
                
    for(var i = 0; i < FIRE_WIDTH; i++) {
      firePixels[(FIRE_HEIGHT-1)*FIRE_WIDTH + i] = 36;
      firePixelsToFireBuffer((FIRE_HEIGHT-1)*FIRE_WIDTH + i);
    }
  };
            
  var spreadFire = function (pixel, curSrc, counter, srcOffset, rand, width) {
    if(pixel != 0) {
      var randIdx = Math.round(Math.random() * 255.0) & 255;
      var tmpSrc;
                    
      rand = ((rand+2) & 255);
      tmpSrc = (curSrc + (((counter - (randIdx & 3)) + 1) & (FIRE_WIDTH - 1)));
      firePixels[tmpSrc - FIRE_WIDTH] = pixel - ((randIdx & 1));
      firePixelsToFireBuffer(tmpSrc - FIRE_WIDTH);
    }
    else {
      firePixels[srcOffset - FIRE_WIDTH] = 0;
      firePixelsToFireBuffer(srcOffset - FIRE_WIDTH);
    }
                
    return rand;
  };
            
  var doFire = function () {
    var counter = 0;
    var curSrc = 0;
    var srcOffset = 0;
    var rand = 0;
    var step = 0;
    var pixel = 0;
    var i = 0;
               
    rand = Math.round(Math.random() * 255.0) & 255;
    curSrc = FIRE_WIDTH;
                
    do {
      srcOffset = (curSrc + counter);
      pixel = firePixels[srcOffset];
      step = 2;
                    
      rand = spreadFire(pixel, curSrc, counter, srcOffset, rand, FIRE_WIDTH);
                    
      curSrc += FIRE_WIDTH;
      srcOffset += FIRE_WIDTH;
                    
      do {
        pixel = firePixels[srcOffset];
        step += 2;
                        
        rand = spreadFire(pixel, curSrc, counter, srcOffset, rand, FIRE_WIDTH);
                        
        pixel = firePixels[srcOffset + FIRE_WIDTH];
        curSrc += FIRE_WIDTH;
        srcOffset += FIRE_WIDTH;
                        
        rand = spreadFire(pixel, curSrc, counter, srcOffset, rand, FIRE_WIDTH);
                        
        curSrc += FIRE_WIDTH;
        srcOffset += FIRE_WIDTH;             
      } while(step < FIRE_HEIGHT);
                    
      counter++;
      curSrc -= ((FIRE_WIDTH*FIRE_HEIGHT)-FIRE_WIDTH);
                    
    } while(counter < FIRE_WIDTH);
  };
  
  var tick = function () {
    doFire();
    Video.uploadFramebuffer(fireBuffer);
  };
  
  try {
    init();
    start();
  
    while(1) {
      var evt;
      while((evt = Events.pollEvent()))
      {
        switch(evt.type)
        {
        case SDL.QUIT:
          Core.exit();
          return;
        default:
          break;
        }
      }
     
      tick();    
      render();
      Video.swapBuffers();
      Timer.delay(10);
    }
  }
  catch(err) {
    Console.println("Caught exception in main loop: " + err);
    Core.exit();
  }
};