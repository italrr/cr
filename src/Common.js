const isNodeJS = typeof window === 'undefined';

String.prototype.removeAt = function(index, charcount){
    return this.substring(0, index) + this.substring(index + charcount);
};

const LogType = {
    STDOUT: "STDOUT",
    STDERR: "STDERR"    
};

const ResulType = {
    SUCCESS: "SUCCESS",
    FAILURE: "FAILURE",
    PENDING: "PENDING"
};

const Vec2 = function(x = 0, y = 0){
    return {
        x,
        y,        
        sum: function(vec2){
            return Vec2(this.x + vec2.x, this.y + vec2.y);
        },
        rest: function(vec2){
            return Vec2(this.x - vec2.x, this.y - vec2.y);
        },        
        mult: function(vec2){
            return Vec2(this.x * vec2.x, this.y * vec2.y);
        },
        div: function(vec2){
            return Vec2(this.x / vec2.x, this.y / vec2.y);
        },
        avg: function(){
            return (this.x + this.y) * 0.5;
        },       
        str: function(){
            return `{${this.x}, ${this.y}}`
        },
        set: function(vec2){
            this.x = vec2.x;
            this.y = vec2.y;
            return this;
        }
    };
};

const Color = function(r = 0, g = 0, b = 0, a = 1){
    const me = this;
    return {
        r,
        g,
        b,
        a,        
        sum: function(color){
            return me.Color(    this.r + color.r,
                                this.g + color.g,
                                this.b + color.b);
        },
        rest: function(color){
            return me.Color(    this.r - color.r,
                                this.g - color.g,
                                this.b - color.b);
        }, 
        mult: function(color){
            return me.Color(    this.r * color.r,
                                this.g * color.g,
                                this.b * color.b);
        },
        div: function(color){
            return me.Color(    this.r / color.r,
                                this.g / color.g,
                                this.b / color.b);
        },   
        avg: function(){
            return (this.r + this.g + this.b) / 3;
        },   
        hex: function(){
            const componentToHex = function(c){
                var hex = c.toString(16);
                return hex.length == 1 ? "0" + hex : hex;
            };
            const red = this.r * 255.0;
            const green = this.g * 255.0;
            const blue = this.b * 255.0;
            return '#' + componentToHex(red) + componentToHex(green) + componentToHex(blue);      
        },
        str: function(){
            return `{${this.r}, ${this.g}, ${this.b}, ${this.a}}`;
        },
        setHex: function(str){
            const hexValue = parseInt(str, 16);
            r = ((hexValue >> 16) & 0xFF) / 255.0;
            g = ((hexValue >> 8) & 0xFF) / 255.0;
            b = ((hexValue) & 0xFF) / 255.0;
            a = 1;
            return this;
        },
        set: function(color){
            this.r = color.r;
            this.g = color.g;
            this.b = color.b;
            this.a = color.a;
            return this;
        }
    };
}


const Result = function(status, reason, payload = null){
    return {
        status: type,
        reason,
        payload
    };
};

const Tools = {   
    componentToHex: function(c){
        var hex = c.toString(16);
        return hex.length == 1 ? "0" + hex : hex;
    },
    getTicks: function(){
        return Date.now();
    },
    abs: function(num1, num2){
        if(num1 > num2){
            return num1 - num2;
        }else{
            return num2 - num1;
        }     
    },
    log: function(msg, type = LogType.STDOUT) {
        const me = this;
        if(type == LogType.STDOUT){
            console.log(msg);
        }else
        if(type == LogType.STDERR){
            console.error("\033[31m"+msg+"\033[39m");
        }
    },        
    uuidv4: function(){
        if(!isNodeJS){
            return "";
        }
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
          var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
          return v.toString(10);
        });
    },
    post: function(url, cb, data = null){
        let xhr = new XMLHttpRequest();
        xhr.open("POST", url);
        xhr.setRequestHeader("Accept", "application/json");
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.onload = () => {
            cb(xhr.status, xhr.response);
        }    
        xhr.send(JSON.stringify(data));        
    },
    get: function(url, cb, async = true){
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if (xhr.readyState == XMLHttpRequest.DONE) {
                cb(xhr.status, xhr.response);
            }
        }
        xhr.open('GET', url, async);
        xhr.send(null);
        return async ? null : {status: xhr.status, response: xhr.response};
    },    
    loadShaderContent: function(vert, frag){
        const me = this;
        let vertSource = null;
        let fragSource = null;
        const vertResult = me.get(`/src/cl/data/shaders/${vert}.glsl`, function(){}, false);
        const fragResult = me.get(`/src/cl/data/shaders/${frag}.glsl`, function(){}, false);
        if(vertResult){
            if(vertResult.status != 200){
                me.log(`Failed to load Shader: Vert ${vert}`, LogType.STDERR);
            }
            
            vertSource = vertResult.response;
        }
        if(fragResult){
            if(fragResult.status != 200){
                me.log(`Failed to load Shader: Frag ${frag}`, LogType.STDERR);
            }
            fragSource = fragResult.response;
        }        
        return {vert: vertSource, frag: fragSource};
    }    
};

if(isNodeJS){
    module.exports = {
        Vec2,
        Color,
        Result,
        Tools
    };
}