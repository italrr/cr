
const RenderEngine = function(canvas){

    const width = canvas.clientWidth;
    const height = canvas.clientHeight;

    canvas.width = width;
    canvas.height = height;

    return {
        gl: canvas.getContext("webgl"),
        canvas,
        width,
        height,
        init: function(){
   
        },
        setLoop: function(stepFunct, drawFunct){
            const me = this;
            const finalLoop = function(time){
                stepFunct(time);
                drawFunct(time);
                requestAnimationFrame(finalLoop);
            };
            requestAnimationFrame(finalLoop);
        },
    };
};