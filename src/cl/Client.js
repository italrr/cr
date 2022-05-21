

window.onload = function(){
    
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);


    const shader = Tools.loadShaderContent("v_shader", "f_shader");


    setTimeout(function(){
        const renderer = RenderEngine(document.getElementById("canvas-game"));

        renderer.init();
    
    
        const programInfo = twgl.createProgramInfo(renderer.gl, [shader.vert, shader.frag]);
        const arrays = {
            position: [-1, -1, 0, 1, -1, 0, -1, 1, 0, -1, 1, 0, 1, -1, 0, 1, 1, 0],
          };
        const bufferInfo = twgl.createBufferInfoFromArrays(renderer.gl, arrays);
    
    
        renderer.setLoop(
            // STEP
            function(time){
                
                
            },
            // DRAW
            function(time){
    
                // twgl.resizeCanvasToDisplaySize(renderer.gl.canvas);
                renderer.gl.viewport(0, 0, renderer.width, renderer.height);
            
                const uniforms = {
                  time: time * 0.001,
                  resolution: [renderer.width, renderer.height],
                };
            
                renderer.gl.useProgram(programInfo.program);
                twgl.setBuffersAndAttributes(renderer.gl, programInfo, bufferInfo);
                twgl.setUniforms(programInfo, uniforms);
                twgl.drawBufferInfo(renderer.gl, bufferInfo);
    
            }
        );
        
    }, 200)


};