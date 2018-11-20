sdlTest = function () {
  var rotation = 0.0;
  
  var init = function () {
    Video.openWindow();
    Video.setCaption("vibconsole");

    gl.MatrixMode(gl.MODELVIEW);
    gl.LoadIdentity();
    gl.MatrixMode(gl.PROJECTION);
    gl.LoadIdentity();
    gl.Ortho(0.0, 640.0, 480.0, 0.0, -1.0, 1.0);
    gl.Disable(gl.DEPTH_TEST);
    gl.ClearColor(0.0, 0.0, 0.0, 0.0);
    gl.Clear(gl.COLOR_BUFFER_BIT);
  };
  
  var tick = function () {
    rotation += 0.05;
  };
  
  var render = function () {
    gl.Clear(gl.COLOR_BUFFER_BIT);
    gl.MatrixMode(gl.MODELVIEW);
    gl.PushMatrix();
    gl.Translated(320.0, 240.0, 0.0);
    gl.Rotated(rotation, 0.0, 0.0, 1.0);
    gl.Translated(-320.0, -240.0, 0.0);
    gl.Begin(gl.QUADS);
      gl.Color3d(1.0, 0.0, 0.0);
      gl.Vertex2d(160.0, 120.0);
      gl.Vertex2d(160.0, 360.0);
      gl.Vertex2d(480.0, 360.0);
      gl.Vertex2d(480.0, 120.0);
    gl.End();
    gl.PopMatrix();
  }; 
  
  init();
  
  while(1) {
    var evt;
    while((evt = Events.pollEvent()))
    {
      switch(evt.type)
      {
      case SDL.QUIT:
        Core.exit();
        return;
      case SDL.KEYDOWN:
        Console.println("KEYDOWN");
        Console.println(" key.type  = " + evt.key.type);
        Console.println(" key.state = " + evt.key.state);
        Console.println(" key.keysym.scancode = " + evt.key.keysym.scancode);
        Console.println(" key.keysym.sym      = " + evt.key.keysym.sym);
        Console.println(" key.keysym.mod      = " + evt.key.keysym.mod);
        Console.println(" key.keysym.unicode  = '" + String.fromCharCode(evt.key.keysym.unicode) + "'");
        break;
      default:
        Console.println(evt.type);
        break;
      }
    }
    
    tick();    
    render();
    Video.swapBuffers();
    Timer.delay(10);
  }
};