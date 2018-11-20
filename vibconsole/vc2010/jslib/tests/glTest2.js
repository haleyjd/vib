glTest = function () {
  var rotation = 0.0;
  var quit = false;
  
  var init = function () {
    Video.openWindow();
    Video.setCaption("SDL/GL Quad Rotation Demo");
    Events.enableKeyRepeat();

    gl.MatrixMode(gl.MODELVIEW);
    gl.LoadIdentity();
    gl.MatrixMode(gl.PROJECTION);
    gl.LoadIdentity();
    gl.Ortho(0.0, 640.0, 480.0, 0.0, -1.0, 1.0);

    gl.BlendFunc(gl.SRC_ALPHA_SATURATE, gl.ONE);
    gl.ClearColor(0.0, 0.0, 0.0, 0.0);
    gl.Clear(gl.COLOR_BUFFER_BIT);
    gl.Enable(gl.BLEND);
    gl.Enable(gl.POLYGON_SMOOTH); 
    gl.Disable(gl.DEPTH_TEST);
  };
  
  var render = function () {
    gl.Clear(gl.COLOR_BUFFER_BIT);
    gl.MatrixMode(gl.MODELVIEW);
    gl.PushMatrix();
    gl.Translated(320.0, 240.0, 0.0);
    gl.Rotated(rotation, 0.0, 0.0, 1.0);
    gl.Translated(-320.0, -240.0, 0.0);
    gl.Begin(gl.QUADS);
      gl.Color4d(1.0, 0.0, 0.0, 1.0);
      gl.Vertex2d(160.0, 120.0);
      gl.Vertex2d(160.0, 360.0);
      gl.Vertex2d(480.0, 360.0);
      gl.Vertex2d(480.0, 120.0);
    gl.End();
    gl.PopMatrix();
  }; 
  
  var keyHit = function (event) {
    switch(event.key.keysym.sym)
    {
    case SDLKey.q:
      quit = true;
      break;
    case SDLKey.RIGHT:
      rotation += 1;
      break;
    case SDLKey.LEFT:
      rotation -= 1;
      break;
    default:
      break;
    }
  }
  
  init();
  
  while(!quit) {
    var evt;
    while((evt = Events.pollEvent()))
    {
      switch(evt.type)
      {
      case SDL.QUIT:
        quit = true;
        break;
      case SDL.KEYDOWN:
        keyHit(evt);
        break;
      default:
        break;
      }
    }
     
    render();
    Video.swapBuffers();
    Timer.delay(10);
  }
  
  SDL.quit();
};