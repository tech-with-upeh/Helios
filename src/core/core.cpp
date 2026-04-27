#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN  // Strips bloat from windows.h
#define NOMINMAX             // Prevents conflicts with std::min/max
#define POPEN _popen
#define PCLOSE _pclose
#include <windows.h>   
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

#include <filesystem>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <thread> 


#include "lexer.hpp"
#include "parser.hpp"
#include "astvisualise.hpp"
#include "preprocessor.hpp"
#include "semantics.hpp"
#include "web_engine.hpp"
#include "core.hpp"
#include "utils.hpp"


#include <memory>
#include <uWebSockets/App.h>
#include "httpserver.hpp"



namespace fs = std::filesystem;


Core::Core() {}

std::string Core::getProjectRoot(const std::string& root, bool useroot) {
    std::string file;
    if(useroot) {
        file = root + "/helios.config";
    } else {
        file = "helios.config";
    }
    ifstream config(file);
    std::string line;
    if (config.is_open()) {
        while (getline(config, line)) {
            if (line.rfind("project_root=", 0) == 0) {
                return line.substr(strlen("project_root="));
            }
        }
    } else {
        cerr << "Error: Couldnt find a config file to run!'\n";
        exit(1);
    }
    return "MyApp";
}

void Core::saveProjectRoot(const std::string& root) {
    ofstream config(root+ "/helios.config");
    config << "project_root=" << root << "\n";
}

void Core::generateFiles(const std::vector<std::string>& targets, const std::string& pname) {
    std::string root = getProjectRoot(pname, true);
    if (!fs::exists(root)) {
        cerr << "Trying to access root but doesnt exist" << endl;
        exit(1);
    };

    fs::create_directory(root + "/public");
    ofstream(root + "/index.ink") << R"(
        page("My APP") {
            @state num : 0
            view("mydiv", style={
                "height": "50px",
                "background-color": "green"
            }, onclick=(num) {
                num = num + 1
                print(num)
            }) {
                text('Click me and check console!')
            } 
        }
        )";
    for (auto& target : targets) {
        if (target == "web") {
            fs::create_directory(root + "/web");
            ofstream(root + "/web/index.html") << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Helios App</title>
</head>
                <body>
                    <script>
                        window.Helios = {
                            ws: null,

                            init() {
                                const protocol = location.protocol === "https:" ? "wss" : "ws";
                                this.ws = new WebSocket(`${protocol}://${location.host}`);

                                this.ws.onmessage = (e) => {
                                    if (e.data === "reload") location.reload();
                                };
                            },

                            send(msg) {
                                if (this.ws?.readyState === WebSocket.OPEN) {
                                    this.ws.send(msg);
                                }
                            }
                        };

                        Helios.init();
                        var Module = {
                            onRuntimeInitialized: function() {
                                console.log('[HELIOS] WASM Module initialized [HELIOS]');
                            },
                            print: function(text) {
                                Helios.send(text);
                                console.log('[HELIOS]:', text);
                            }
                        };

                        document.addEventListener('click', function(event) {
                            const target = event.target;
                            const callbackId = target.getAttribute('data-callback');
                            
                            if (callbackId && Module && Module._invokeVNodeCallback) {
                                const length = Module.lengthBytesUTF8(callbackId) + 1;
                                const buffer = Module._malloc(length);
                                Module.stringToUTF8(callbackId, buffer, length);
                                
                                Module._invokeVNodeCallback(buffer);
                                Module._free(buffer);
                            }
                        });
                    </script>
                    
                    <script src="main.js"></script>
                </body>
                </html>)";
            ofstream(root + "/web/vdom.hpp") << R"(#pragma once
                #include <emscripten.h>
                #include <string>
                #include <cstring>
                #include <vector>
                #include <unordered_map>
                #include <sstream>
                #include <functional>
                #include <algorithm>

                // -------------------- Forward declarations --------------------
                struct VPage;
                struct VNode;
                void renderPage(VPage& page, bool statechange=false, bool isInternalPage = false);
                std::string genId();
                void diff(const VNode& oldN, const VNode& newN);
                extern "C" {
                    EMSCRIPTEN_KEEPALIVE
                    void js_removeInlineCSS(const char* key);
                }

                struct Patch {
                    enum Type {
                        SET_TEXT,
                        SET_ATTR,
                        REMOVE_ATTR,
                        INSERT_HTML,
                        REMOVE_NODE
                    };

                    Type type;
                    std::string id;
                    std::string key;
                    std::string value;
                };

                static std::vector<Patch> patches;

                inline void queueSetText(const std::string& id,const std::string& text){
                    patches.push_back({Patch::SET_TEXT,id,"",text});
                }

                inline void queueSetAttr(const std::string& id,const std::string& k,const std::string& v){
                    patches.push_back({Patch::SET_ATTR,id,k,v});
                }

                inline void queueRemoveAttr(const std::string& id,const std::string& k){
                    patches.push_back({Patch::REMOVE_ATTR,id,k,""});
                }

                inline void queueRemoveNode(const std::string& id){
                    patches.push_back({Patch::REMOVE_NODE,id,"",""});
                }

                inline void queueInsertHTML(const std::string& id,const std::string& html){
                    patches.push_back({Patch::INSERT_HTML,id,"",html});
                }

                inline void applyPatches()
                {
                    for(auto& p:patches)
                    {
                        switch(p.type)
                        {
                            case Patch::SET_TEXT:
                                EM_ASM({
                                    const el = Module.domCache[UTF8ToString($0)];
                                    if(el) el.textContent = UTF8ToString($1);
                                },p.id.c_str(),p.value.c_str());
                                break;

                            case Patch::SET_ATTR:
                                EM_ASM({
                                    const el = Module.domCache[UTF8ToString($0)];
                                    if(el) el.setAttribute(UTF8ToString($1),UTF8ToString($2));
                                },p.id.c_str(),p.key.c_str(),p.value.c_str());
                                break;

                            case Patch::REMOVE_ATTR:
                                EM_ASM({
                                    const el = Module.domCache[UTF8ToString($0)];
                                    if(el) el.removeAttribute(UTF8ToString($1));
                                },p.id.c_str(),p.key.c_str());
                                break;

                            case Patch::INSERT_HTML:
                                EM_ASM({
                                    const parent = Module.domCache[UTF8ToString($0)];
                                    if(parent){
                                        parent.insertAdjacentHTML("beforeend",UTF8ToString($1));
                                        parent.querySelectorAll("[data-ink-id]").forEach(el=>{
                                            Module.domCache[el.dataset.inkId]=el;

                                            // Attach onclick if data-callback exists
                                            const cbId = el.dataset.callback;
                                            if(cbId) {
                                                el.onclick = () => Module._invokeVNodeCallback(cbId);
                                            }
                                        });
                                    }
                                },p.id.c_str(),p.value.c_str());
                                break;

                            case Patch::REMOVE_NODE:
                                EM_ASM({
                                    const el = Module.domCache[UTF8ToString($0)];
                                    if(el) el.remove();
                                },p.id.c_str());
                                break;
                        }
                    }

                    patches.clear();
                }

                // -------------------- Global Page State --------------------
                namespace GlobalState {
                    static VPage* currentPage = nullptr;

                    static void setCurrentPage(VPage* page) {
                        currentPage = page;
                    }

                    static VPage* getCurrentPage() {
                        return currentPage;
                    }

                }

                // -------------------- Proper State Management --------------------
                namespace appstate {
                    template<typename T>
                    class State {
                    private:
                        std::string key;

                    public:
                        State(const std::string& k, T initial) : key(k) {
                            // Force initialization in JS storage
                            initialize(initial);
                        }

                        void initialize(T initial_value) {
                            if constexpr (std::is_same_v<T, int>) {
                                EM_ASM({
                                    window.wasmState = window.wasmState || {};
                                    // Only initialize if not already set
                                    if (window.wasmState[UTF8ToString($0)] === undefined) {
                                        window.wasmState[UTF8ToString($0)] = $1;
                                    }
                                }, key.c_str(), initial_value);
                            } else if constexpr (std::is_same_v<T, std::string>) {
                                EM_ASM({
                                    window.wasmState = window.wasmState || {};
                                    if (window.wasmState[UTF8ToString($0)] === undefined) {
                                        window.wasmState[UTF8ToString($0)] = UTF8ToString($1);
                                    }
                                }, key.c_str(), initial_value.c_str());
                            } else if constexpr (std::is_same_v<T, double>) {
                                EM_ASM({
                                    window.wasmState = window.wasmState || {};
                                    if (window.wasmState[UTF8ToString($0)] === undefined) {
                                        window.wasmState[UTF8ToString($0)] = $1;
                                    }
                                }, key.c_str(), initial_value);
                            }
                        }

                        void set(T new_value) {
                            if constexpr (std::is_same_v<T, int>) {
                                EM_ASM({
                                    window.wasmState = window.wasmState || {};
                                    window.wasmState[UTF8ToString($0)] = $1;
                                }, key.c_str(), new_value);
                            } else if constexpr (std::is_same_v<T, std::string>) {
                                EM_ASM({
                                    window.wasmState = window.wasmState || {};
                                    window.wasmState[UTF8ToString($0)] = UTF8ToString($1);
                                }, key.c_str(), new_value.c_str());
                            } else if constexpr (std::is_same_v<T, double>) {
                                EM_ASM({
                                    window.wasmState = window.wasmState || {};
                                    window.wasmState[UTF8ToString($0)] = $1;
                                }, key.c_str(), new_value);
                            }
                        }

                        T get() const {
                            if constexpr (std::is_same_v<T, int>) {
                                return EM_ASM_INT({
                                    window.wasmState = window.wasmState || {};
                                    var val = window.wasmState[UTF8ToString($0)];
                                    // Return default if not initialized
                                    return (val === undefined) ? 0 : val;
                                }, key.c_str());
                            } else if constexpr (std::is_same_v<T, std::string>) {
                                char* result = (char*)EM_ASM_INT({
                                    window.wasmState = window.wasmState || {};
                                    var val = window.wasmState[UTF8ToString($0)];
                                    if (val === undefined) {
                                        val = ""; // Default value
                                    }
                                    var length = lengthBytesUTF8(val) + 1;
                                    var buffer = _malloc(length);
                                    stringToUTF8(val, buffer, length);
                                    return buffer;
                                }, key.c_str());

                                if (result) {
                                    std::string str(result);
                                    free(result);
                                    return str;
                                }
                                return "";
                            } else if constexpr (std::is_same_v<T, double>) {
                                return EM_ASM_DOUBLE({
                                    window.wasmState = window.wasmState || {};
                                    var val = window.wasmState[UTF8ToString($0)];
                                    return (val === undefined) ? 0.0 : val;
                                }, key.c_str());
                            }
                            return T();
                        }
                    };
                }




                // -------------------- Callback Registry --------------------
                class CallbackRegistry {
                    private:
                        static std::unordered_map<std::string, std::function<void()>> callbacks;
                        static int nextId;

                    public:
                        static std::string registerCallback(std::function<void()> callback) {
                            std::string id = "callback_" + std::to_string(nextId++);
                            callbacks[id] = callback;
                            return id;
                        }

                        static void invokeCallback(const std::string& id) {
                            auto it = callbacks.find(id);
                            if (it != callbacks.end()) {
                                it->second();
                            }
                        }
                };

                std::unordered_map<std::string, std::function<void()>> CallbackRegistry::callbacks;
                int CallbackRegistry::nextId = 0;

                // -------------------- VNode --------------------
                enum class VNodeType {
                    NORMAL,
                    CANVAS
                };

                struct VNode {

                    VNodeType type = VNodeType::NORMAL;

                    int width = 300;
                    int height = 150;
                    std::string canvasid = "main-canvas";

                    std::string tag;
                    std::string dom_id;
                    std::string id;
                    std::string text;

                    std::string key;

                    std::vector<VNode> children;
                    std::unordered_map<std::string,std::string> attrs;

                    std::function<void()> onclick;
                    std::string callback_id;

                    VNode() = default;

                    VNode(std::string t, std::string txt="", std::string_view ink_domid = "")
                        : tag(t), text(txt)
                    {
                        if (ink_domid.empty())
                            dom_id = genId();
                        else
                            dom_id = ink_domid;

                        key = dom_id;   // automatic key
                    }

                    VNode& setKey(const std::string& k){
                        key=k;
                        return *this;
                    }

                    VNode& setText(const std::string& t){
                        text=t;
                        return *this;
                    }

                    VNode& setAttr(const std::string& k,const std::string& v){
                        attrs[k]=v;
                        return *this;
                    }

                    VNode& addChild(const VNode& c){
                        children.push_back(c);
                        return *this;
                    }

                    VNode& onClick(std::function<void()> fn){
                        onclick=fn;
                        return *this;
                    }
                };
                // -------------------- VPage --------------------
                struct VPage {
                    std::string title;
                    std::vector<VNode> children;
                    std::vector<VNode> old_children; 
                    std::unordered_map<std::string, std::string> bodyAttrs;
                    std::unordered_map<std::string, std::string> stylesheet;

                    std::function<void(VPage&, std::string msg)> builder; 
                    std::vector<std::function<void()>> onMount_list;
                    bool reqanimate = false;
                    std::function<void()> onanimate;
                    std::unordered_map<std::string, std::function<void()>> page_callbacks;

                    std::string favicon;
                    std::vector<std::string> scripts;
                    std::vector<std::string> stylesheets;
                    std::vector<std::string> old_scripts;
                    std::vector<std::string> old_stylesheets;
                    std::string old_favicon;




                    // Helper methods
                    VPage& setTitle(const std::string& newTitle) {
                        title = newTitle;
                        return *this;
                    }

                    VPage& addStyle(const std::string& key, const std::string& newstylesheet) {
                        stylesheet[key] = newstylesheet;
                        return *this;
                    }

                    VPage& removeStyle(const std::string& key) {
                        js_removeInlineCSS(key.c_str());
                        stylesheet.erase(key);
                        return *this;
                    }

                    VPage& addChild(const VNode& child) {
                        children.push_back(child);
                        return *this;
                    }

                    VPage& clearChildren() {
                        children.clear();
                        onMount_list.clear();
                        stylesheet.clear();
                        scripts.clear();
                        stylesheets.clear();
                        return *this;
                    }

                    VPage& rebuild(std::string msg = "") {
                        if (!builder) return *this;
                        old_children = children;
                        old_scripts = scripts;
                        old_stylesheets = stylesheets;
                        old_favicon = favicon;
                        clearChildren();
                        builder(*this, msg);

                        return *this;
                    }

                    // Render this page
                    void render(bool statechange=false) {
                        rebuild();
                        renderPage(*this, statechange);

                    }
                    void onMount(std::function<void()> fn) {
                        onMount_list.push_back(fn);
                    }

                    void onAnimatefps(std::function<void()> fn) {
                        reqanimate = true;
                        onanimate = fn;
                    }
                    void addevent(std::string event, std::function<void()> fn) {
                        page_callbacks[event] = fn;
                    }

                    void addScript(const std::string& src) {
                        scripts.push_back(src);
                    }

                    void addStylesheet(const std::string& href) {
                        stylesheets.push_back(href);
                    }

                    void setFavicon(const std::string& link) {
                        favicon = link;
                    }
                };


                class Canvas2D {
                    std::string id;
                    public:
                    Canvas2D(std::string canvasId) : id(canvasId) {}

                    void clear() {
                        EM_ASM({
                            if (document.getElementById($0)) {
                                const ctx = document.getElementById(UTF8ToString($0)).getContext("2d");
                                ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
                            }

                        }, id.c_str());
                    }

                    // if i get lost

                    void setFill(const std::string& color) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.fillStyle = UTF8ToString($1);
                        }, id.c_str(), color.c_str());
                    }

                    void setStroke(const std::string& color) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.strokeStyle = UTF8ToString($1);
                        }, id.c_str(), color.c_str());
                    }

                    void lineWidth(size_t w) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.lineWidth = $1;
                        }, id.c_str(), w);
                    }

                    void alpha(double a) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.globalAlpha = $1;
                        }, id.c_str(), a);
                    }

                    // ─────────────────────────────
                    // Shapes
                    // ─────────────────────────────

                    void rect(size_t x, size_t y, size_t w, size_t h) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.fillRect($1, $2, $3, $4);
                        }, id.c_str(), x, y, w, h);
                    }

                    void strokeRect(size_t x, size_t y, size_t w, size_t h) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.strokeRect($1, $2, $3, $4);
                        }, id.c_str(), x, y, w, h);
                    }

                    void line(size_t x1, size_t y1, size_t x2, size_t y2) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.beginPath();
                            ctx.moveTo($1, $2);
                            ctx.lineTo($3, $4);
                            ctx.stroke();
                        }, id.c_str(), x1, y1, x2, y2);
                    }

                    void circle(size_t x, size_t y, size_t r) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.beginPath();
                            ctx.arc($1, $2, $3, 0, Math.PI * 2);
                            ctx.fill();
                        }, id.c_str(), x, y, r);
                    }

                    void strokeCircle(size_t x, size_t y, size_t r) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.beginPath();
                            ctx.arc($1, $2, $3, 0, Math.PI * 2);
                            ctx.stroke();
                        }, id.c_str(), x, y, r);
                    }

                    // ─────────────────────────────
                    // Text
                    // ─────────────────────────────

                    void font(const std::string& f) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.font = UTF8ToString($1);
                        }, id.c_str(), f.c_str());
                    }

                    void text(const std::string& t, size_t x, size_t y) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.fillText(UTF8ToString($1), $2, $3);
                        }, id.c_str(), t.c_str(), x, y);
                    }

                    // ─────────────────────────────
                    // Transforms
                    // ─────────────────────────────

                    void move(size_t x, size_t y) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.translate($1, $2);
                        }, id.c_str(), x, y);
                    }

                    void rotate(double r) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.rotate($1);
                        }, id.c_str(), r);
                    }

                    void scale(double x, double y) {
                        EM_ASM({
                            const ctx = document.getElementById(UTF8ToString($0))?.getContext("2d");
                            if (!ctx) return;
                            ctx.scale($1, $2);
                        }, id.c_str(), x, y);
                    }
                };

                class Platform {
                    public:
                        double height() {
                            return EM_ASM_DOUBLE({
                                console.log(document.body.getBoundingClientRect().height);
                                return document.body.getBoundingClientRect().height;
                            });
                        }
                        double width() {
                            return EM_ASM_DOUBLE({
                                    return document.body.getBoundingClientRect().width;
                                });
                        }
                        double scrollY() {
                            return EM_ASM_DOUBLE({
                                    return window.scrollY;
                                });
                        }
                };

                std::shared_ptr<VPage> MakeErrorPage() {
                    auto ErrorPage = std::make_shared<VPage>();
                    ErrorPage->builder = [&](VPage& page, std::string msg) {
                        page.setTitle("Helios Error");
                        page.addStylesheet("https://cdn.lineicons.com/5.1/line/lineicons.css");
                        page.setFavicon("logo.png");
                        page.bodyAttrs["style"] = "background-color:black;color:white;padding:0px;margin:0px;";


                        VNode ambientWrapper("div", "", "__ink_ambientWrapper");
                        ambientWrapper.setAttr("id", "ambient-glow");
                        ambientWrapper.setAttr("style",
                            "position:absolute;top:0;left:0;width:100%;height:100%;"
                            "overflow:hidden;pointer-events:none;z-index:0;"
                        );

                        // Top-left deep purple glow
                        VNode glow1("div", "", "__ink_glow1");
                        glow1.setAttr("style",
                            "position:absolute;top:-10%;left:-10%;width:50%;height:50%;"
                            "background-color:rgb(41,8,128);"
                            "border-radius:50%;filter:blur(120px);opacity:0.6;"
                        );

                        // Top-right primary accent glow
                        VNode glow2("div", "", "__ink_glow2");
                        glow2.setAttr("style",
                            "position:absolute;top:40%;right:-10%;width:60%;height:60%;"
                            "background-color:rgb(114,52,248);"
                            "border-radius:50%;filter:blur(140px);opacity:0.2;"
                        );

                        // Bottom-left highlight glow
                        VNode glow3("div", "", "__ink_glow3");
                        glow3.setAttr("style",
                            "position:absolute;bottom:-10%;left:20%;width:70%;height:40%;"
                            "background-color:rgb(84,21,255);"
                            "border-radius:50%;filter:blur(100px);opacity:0.2;"
                        );

                        ambientWrapper.addChild(glow1);
                        ambientWrapper.addChild(glow2);
                        ambientWrapper.addChild(glow3);

                        
                        
                        VNode view_56("div", "", "__ink_29");
                        view_56.setAttr("id", "error-wrapper");
                        view_56.setAttr("style", "height:100%;width:100%;display:flex;justify-content:center;");

                        VNode view_57("div", "", "__ink_30");
                        view_57.setAttr("id", "err-main");
                        view_57.setAttr("style", "width:80%;display:flex;flex-direction:column;justify-content:flex-start;align-items:center;padding:20px;background-color:rgb(114, 52, 230);height:80%;align-self:center;border-radius:20px;");

                        VNode view_58("div", "", "__ink_31");
                        view_58.setAttr("id", "err-start");

                        VNode text_59("p","Error on FIle", "__ink_32");

                        view_58.addChild(text_59);
                        view_57.addChild(view_58);
                        VNode view_62("div", "", "__ink_33");
                        view_62.setAttr("id", "err-hr");
                        view_62.setAttr("style", "height:1px;;width:100%;background-color:#333;;margin:10px 0;;");

                        view_57.addChild(view_62);
                        VNode view_64("div", "", "__ink_34");
                            view_64.setAttr("id", "err-end");
                        view_64.setAttr("style", "height:100%;width:100%;display:flex;justify-content:center;align-items:center;");

                        VNode text_65("p", msg, "__ink_35");
                        view_64.setAttr("style", "white-space:pre-wrap;text-align:center;color:#eee;font-family:monospace;");

                        view_64.addChild(text_65);
                        view_57.addChild(view_64);
                        view_56.addChild(view_57);
                        page.addChild(ambientWrapper);
                        page.addChild(view_56);
                    };
                    return ErrorPage;
                }

                // --------------------Router ------------------------------
                class Router {
                    public:
                    using Handler = std::shared_ptr<VPage>; // store shared_ptr to avoid copies

                    // Add a route
                    static void add(const std::string& path, Handler handler) {
                        routes()[path] = handler;
                    }

                    // Navigate to a path
                    static void navigate(const std::string& path, bool iserr) {
                        if(iserr == true) {
                            renderPage(MakeErrorPage()->rebuild(path), true, true);
                        }else {
                            auto it = routes().find(path);
                            if (it != routes().end() && it->second) {
                                currentPath() = path;
                                it->second->render();
                            } else {
                                get404()->render();
                            }
                        }
                    }

                    static void go(const std::string& path, bool push = true) {
                        auto it = routes().find(path);
                        if (it != routes().end()) {
                            currentPath() = path;
                            GlobalState::setCurrentPage(&(*(it->second)));
                            it->second->render(); 

                            if (push) {
                                EM_ASM({
                                    history.pushState({}, "", UTF8ToString($0));
                                }, path.c_str());
                            }
                        } else {
                            get404()->rebuild();
                        }
                    }

                    // Get the current path
                    static std::string getCurrentPath() {
                        return currentPath();
                    }

                    private:
                    // Route map
                    static std::unordered_map<std::string, Handler>& routes() {
                        static std::unordered_map<std::string, Handler> r;
                        return r;
                    }

                    // Current path
                    static std::string& currentPath() {
                        static std::string p;
                        return p;
                    }
                    

                    // 404 page
                    static Handler get404() {
                        static Handler notfound;
                        std::string notfound_path = "/notfound";
                        auto it = routes().find(notfound_path);
                        if (it != routes().end() && it->second) {
                            notfound = [it]() {

                                return it->second;
                            }();
                        } else {
                            notfound = []() {
                                auto page = std::make_shared<VPage>();
                                page->setTitle("Page Not Found");
                                page->addChild(VNode("p", "Page not found"));
                                return page;
                            }();
                        }

                        return notfound;
                    }
                };

                // -------------------- JavaScript Interop --------------------
                extern "C" {
                    EMSCRIPTEN_KEEPALIVE
                    void invokeVNodeCallback(const char* callbackId) {
                        std::string id(callbackId);
                        CallbackRegistry::invokeCallback(id);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_insertHTML(const char* html) {
                        EM_ASM({
                            document.body.style = allocateUTF8(""); 
                            document.body.innerHTML = UTF8ToString($0);
                        }, html);
                        EM_ASM({
                            document.querySelectorAll("[data-ink-id]").forEach(el => {
                                Module.domCache[el.dataset.inkId] = el;
                            });
                        });
                        EM_ASM({
                            document.querySelectorAll("[data-ink-id]").forEach(el => {
                                Module.domCache[el.dataset.inkId] = el;
                                const cbId = el.dataset.callback;
                                if(cbId) {
                                    el.onclick = () => Module._invokeVNodeCallback(cbId);
                                }
                            });
                        });
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void handleRoute(const char* route, bool isErr=false) {
                        Router::navigate(route, isErr);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_setTitle(const char* title) {
                        EM_ASM({
                            document.title = UTF8ToString($0);
                        }, title);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_insertCSS(const char* key, const char* css) {
                        if (strcmp(css, "") != 0){    
                            EM_ASM({
                                if (!document.getElementById("__ink_styles")) {
                                const style = document.createElement("style");
                                style.id = "__ink_styles_" + UTF8ToString($0);
                                style.innerHTML = UTF8ToString($1);
                                document.head.appendChild(style);
                                }
                            }, key, css);
                        }
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_removeInlineCSS(const char* key) {
                        EM_ASM({
                            const style = document.getElementById("__ink_styles_" + UTF8ToString($0));
                            if (style) {
                                style.remove();
                            }
                        }, key);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_addscript(const char* link, bool isjs = false) {
                        EM_ASM({
                            if ($1 == 1){
                                const script = document.createElement("script");
                                script.src = UTF8ToString($0);
                                document.head.appendChild(script);
                            } else {
                                const linkElem = document.createElement("link");
                                linkElem.rel = "stylesheet";
                                linkElem.href = UTF8ToString($0);
                                document.head.appendChild(linkElem);
                            }
                        }, link, isjs);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_removescript(const char* link, bool isjs = false, bool delAll = false) {
                        EM_ASM({
                            const url = UTF8ToString($0);

                            if($2 == 1) {
                                // Remove all matching <script src="...">
                                const scripts = document.querySelectorAll("script[src]");
                                scripts.forEach(s => {
                                    s.remove();
                                });

                                // Remove all matching <link href="...">
                                const links = document.querySelectorAll("link[href]");
                                links.forEach(l => {
                                    l.remove();
                                });
                            } else {
                                if ($1 == 1) {
                                    // Remove <script src="...">
                                    const scripts = document.querySelectorAll("script[src]");
                                    scripts.forEach(s => {
                                        if (s.src === url || s.getAttribute("src") === url) {
                                            s.remove();
                                        }
                                    });
                                } else {
                                    // Remove <link href="...">
                                    const links = document.querySelectorAll("link[href]");
                                    links.forEach(l => {
                                        if (l.href === url || l.getAttribute("href") === url) {
                                            l.remove();
                                        }
                                    });
                                }
                            }
                            
                        }, link, isjs, delAll);
                    }


                    void js_favicon(const char* link) {
                        EM_ASM({
                            const linkElem = document.createElement("link");
                            linkElem.rel = "icon";
                            linkElem.href = UTF8ToString($0);
                            linkElem.type = "image/x-icon";
                            document.head.appendChild(linkElem);
                        }, link);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_setBodyAttr(const char* key, const char* val) {
                        if (strcmp(key, "")) {
                            EM_ASM({
                                document.body.setAttribute(UTF8ToString($0), UTF8ToString($1));
                            }, key, val);
                        }
                    }

                    EMSCRIPTEN_KEEPALIVE
                    char* allocateString(const char* str) {
                        size_t len = strlen(str) + 1;
                        char* buffer = (char*)malloc(len);
                        strcpy(buffer, str);
                        return buffer;
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void freeString(char* str) {
                        free(str);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_mountCanvas(const char* id, const char* html) {
                        EM_ASM({
                            const id = UTF8ToString($0);
                            if (!document.getElementById(id)) {
                                document.body.insertAdjacentHTML("beforeend", UTF8ToString($1));
                            }
                        }, id, html);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void animatefps() {
                        GlobalState::getCurrentPage()->onanimate();
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_reqfps() { 
                        EM_ASM({
                            function rafLoop() {
                                Module._animatefps();
                                requestAnimationFrame(rafLoop);
                            }
                            requestAnimationFrame(rafLoop);
                        });
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void handleEvent(const char* event) {
                        auto it = GlobalState::getCurrentPage()->page_callbacks.find(event);

                        if (it != GlobalState::getCurrentPage()->page_callbacks.end()) {
                            it->second();
                        }
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_addpageEventlisteners(const char* event) {
                        EM_ASM({
                            window.addEventListener(UTF8ToString($0), function () {
                                Module._handleEvent($0);
                            });
                        }, event);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_setText(const char* id, const char* text) {
                        EM_ASM({
                            const el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
                            if (el) el.textContent = UTF8ToString($1);
                        }, id, text);
                    }
                    EMSCRIPTEN_KEEPALIVE
                    void js_setAttr(const char* id, const char* key, const char* val) {
                        EM_ASM({
                            const el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
                            if (!el) return;
                            el.setAttribute(UTF8ToString($1), UTF8ToString($2));
                        }, id, key, val);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_removeAttr(const char* id, const char* key) {
                        EM_ASM({
                            const el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
                            if (!el) return;
                            el.removeAttribute(UTF8ToString($1));
                        }, id, key);
                    }

                    EMSCRIPTEN_KEEPALIVE
                    void js_update_ink_id(const char* oldid, const char* newid) {
                        EM_ASM({
                            el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
                            if (el) {
                                el.attributes['data-ink-id'] = UTF8ToString($1);
                            }

                        }, oldid, newid);
                    }

                }

                inline std::string genId() {
                    static uint64_t id = 0;
                    return "__ink_" + std::to_string(id++);
                }

                // -------------------- Render VNode to HTML --------------------
                inline std::string renderToHTML(const VNode& node) {
                    std::ostringstream oss;
                    if (node.type == VNodeType::CANVAS) {
                        oss << "<canvas"
                            << " width=\"" << node.width << "\""
                            << " height=\"" << node.height << "\"";

                        for (const auto& [k, v] : node.attrs) {
                            oss << " " << k << "=\"" << v << "\"";
                        }

                        oss << "></canvas>";
                        return oss.str();
                    }

                    oss << "<" << node.tag << " data-ink-id=\"" << node.dom_id << "\"";

                    if(node.attrs.find("id") != node.attrs.end()) {
                        oss << " id=\"" << node.attrs.at("id") << "\"";
                    }

                    for(const auto& [k,v] : node.attrs) {
                        if(k != "id") oss << " " << k << "=\"" << v << "\"";
                    }

                    if(!node.callback_id.empty()) {
                        oss << " data-callback=\"" << node.callback_id << "\"";
                    }

                    oss << ">";
                    if(!node.text.empty()) oss << node.text;
                    for(const auto& child : node.children)
                        oss << renderToHTML(child);
                    oss << "</" << node.tag << ">";
                    return oss.str();
                }

                // -------------------- Bind onclick --------------------
                inline void bindOnClick(VNode& node) {
                    if(node.onclick) {
                        node.callback_id = CallbackRegistry::registerCallback(node.onclick);
                    }
                    for(auto& child : node.children)
                        bindOnClick(child);
                }

                inline void diffChildren(const VNode& oldN,const VNode& newN)
                {
                    auto& oldC = oldN.children;
                    auto& newC = newN.children;

                    int oldStart = 0;
                    int newStart = 0;

                    int oldEnd = oldC.size() - 1;
                    int newEnd = newC.size() - 1;

                    while(oldStart <= oldEnd && newStart <= newEnd)
                    {
                        const VNode& oStart = oldC[oldStart];
                        const VNode& oEnd   = oldC[oldEnd];
                        const VNode& nStart = newC[newStart];
                        const VNode& nEnd   = newC[newEnd];

                        if(oStart.key == nStart.key)
                        {
                            diff(oStart,nStart);
                            oldStart++; newStart++;
                            continue;
                        }

                        if(oEnd.key == nEnd.key)
                        {
                            diff(oEnd,nEnd);
                            oldEnd--; newEnd--;
                            continue;
                        }

                        if(oStart.key == nEnd.key)
                        {
                            diff(oStart,nEnd);
                            oldStart++; newEnd--;
                            continue;
                        }

                        if(oEnd.key == nStart.key)
                        {
                            diff(oEnd,nStart);
                            oldEnd--; newStart++;
                            continue;
                        }

                        break;
                    }

                    std::unordered_map<std::string,int> keyMap;

                    for(int i = oldStart; i <= oldEnd; i++)
                        keyMap[oldC[i].key] = i;

                    while(newStart <= newEnd)
                    {
                        auto it = keyMap.find(newC[newStart].key);

                        if(it != keyMap.end())
                        {
                            diff(oldC[it->second], newC[newStart]);
                            keyMap.erase(it);
                        }
                        else
                        {
                            queueInsertHTML(oldN.dom_id, renderToHTML(newC[newStart]));
                        }

                        newStart++;
                    }

                    for(auto& k : keyMap)
                    {
                        queueRemoveNode(oldC[k.second].dom_id);
                    }
                }

                inline void diff(const VNode& oldN,const VNode& newN)
                {
                    if(oldN.tag!=newN.tag)
                    {
                        queueInsertHTML(oldN.dom_id,renderToHTML(newN));
                        queueRemoveNode(oldN.dom_id);
                        return;
                    }

                    if(oldN.text!=newN.text)
                    {
                        queueSetText(oldN.dom_id,newN.text);
                    }

                    for(auto& [k,v]:newN.attrs)
                    {
                        auto it=oldN.attrs.find(k);
                        if(it==oldN.attrs.end()||it->second!=v)
                            queueSetAttr(oldN.dom_id,k,v);
                    }

                    for(auto& [k,v]:oldN.attrs)
                    {
                        if(newN.attrs.find(k)==newN.attrs.end())
                            queueRemoveAttr(oldN.dom_id,k);
                    }

                    diffChildren(oldN,newN);
                }

                // -------------------- Render Page --------------------
                inline void renderPage(VPage& page, bool statechange, bool isInternalPage) {
                    // Set as current page for callbacks
                    GlobalState::setCurrentPage(&page);

                    if (statechange) {

                        if(isInternalPage == true) {
                            js_removescript(page.old_favicon.c_str(), false, true);
                        }

                        for(auto& node : page.children) {
                            bindOnClick(node);
                        }
                        size_t n = std::min(page.old_children.size(), page.children.size());

                        for (size_t i = 0; i < n; i++) {
                            diff(page.old_children[i], page.children[i]);
                        }

                        for (size_t i = n; i < page.children.size(); i++) {
                            auto& child = page.children[i];
                            queueInsertHTML("body", renderToHTML(child));
                            bindOnClick(child);
                        }

                        for (size_t i = n; i < page.old_children.size(); i++) {
                            queueRemoveNode(page.old_children[i].dom_id);
                        }

                        applyPatches();   // ✅ REQUIRED

                        return;
                    }

                    // Initial render
                    std::ostringstream html;
                    std::unordered_map<std::string, std::string> canvas_list;

                    for(auto& node : page.children) {
                        bindOnClick(node);
                        if (node.type == VNodeType::CANVAS) {
                            canvas_list[node.canvasid] = renderToHTML(node);
                            continue;
                        }
                        html << renderToHTML(node);
                    }

                    js_insertHTML(html.str().c_str());

                    if (!page.favicon.empty()) {
                        js_favicon(page.favicon.c_str());
                    }
                    js_setTitle(page.title.c_str());
                    for(const auto& [key, value] : page.stylesheet) {
                        
                        js_insertCSS(key.c_str(), value.c_str());
                    }

                    for(auto& s : page.scripts) {
                        js_addscript(s.c_str(), true);
                    }
                    for(auto& c : page.stylesheets) {
                        js_addscript(c.c_str(), false);
                    }

                    // Mount canvases
                    for (auto &i : canvas_list) {
                        js_mountCanvas(i.first.c_str(), i.second.c_str());
                    }

                    // Body attributes
                    for(const auto& [key, value] : page.bodyAttrs) {
                        js_setBodyAttr(key.c_str(), value.c_str());
                    }

                    // Mount hooks
                    for (auto& fn : page.onMount_list) {
                        fn();
                    }

                    // Animate FPS
                    if (page.reqanimate) {
                        js_reqfps();
                    }

                    // Page-level callbacks
                    for(const auto& k : page.page_callbacks) {
                        js_addpageEventlisteners(k.first.c_str());
                    }
                    applyPatches();
                })";
            ofstream(root + "/web/helios.web.config") << "# placeholder";
            cout << "[web] Generated web folder and files.\n";
        } else if (target == "android") {
            fs::create_directory(root + "/android");
            ofstream(root + "/android/MainActivity.java") << "// placeholder MainActivity";
            cout << "[android] Generated android folder and files.\n";
        } else if (target == "ios") {
            fs::create_directory(root + "/ios");
            cout << "[ios] Generated ios folder.\n";
        } else {
            cout << "[!] Unknown target: " << target << "\n";
        }
    }
}


void Core::builder() {
    ifstream sourcefile("index.ink");
    stringstream buffer;
    char temp;
    while (sourcefile.get(temp))
    {
    buffer << temp; 
    } 
    buffer << ' '; // EOF marker
    std::string sourcecode = buffer.str();
    
    Lexer lexer(sourcecode);
    std::vector<Token *> tokens = lexer.tokenize();
    Parser parser(tokens);
    AST_NODE * root = parser.parse();


    PreProcess preprocessor;
    preprocessor.process(root);


    std::cout << "\n==== AST Visualization ====\n";
    printAST(root);
    std::cout << "\n==== AST Visualization ENDed ====\n";
    cout << "Root Node has " << root->SUB_STATEMENTS.size() << " sub-statements." << endl;
     cout << "[i] Finished Parsing [i]" << endl;

    SemanticAnalyzer analyzer;
    analyzer.analyze(root);
    // cout << "[i] Finished Semantic Analysing [i]" << endl;
    WebEngine gen;
    Core::routes = gen.gen(root);
    //  for (const auto& [url, info] : Core::routes) {
    //     std::cout << url << std::endl;
    //  }

    //TODO:
    //
    #ifdef _WIN32
        std::string cmd = "cmd /c em++ web/generated.cpp -o web/main.js " 
        "-sEXPORTED_FUNCTIONS=\"['_main','_invokeVNodeCallback','_js_insertHTML','_js_setTitle','_malloc','_free', '_handleRoute', '_animatefps', '_handleEvent', '_js_removescript']\" "
        "-sEXPORTED_RUNTIME_METHODS=\"['ccall','cwrap','stringToUTF8','lengthBytesUTF8']\" "
        "-sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -w -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$allocateUTF8' -Q -w -Wfatal-errors 2>&1";
    #else
        std::string cmd = "em++ web/generated.cpp -o web/main.js " 
        "-sEXPORTED_FUNCTIONS=\"['_main','_invokeVNodeCallback','_js_insertHTML','_js_setTitle','_malloc','_free', '_handleRoute', '_animatefps', '_handleEvent', '_js_removescript']\" "
        "-sEXPORTED_RUNTIME_METHODS=\"['ccall','cwrap','stringToUTF8','lengthBytesUTF8']\" "
        "-sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -w -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$allocateUTF8' -Q -w -Wfatal-errors";

    #endif
    // system(cmd.c_str());

    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe) return;

    char ppenbuffer[128];
    while (fgets(ppenbuffer, sizeof(ppenbuffer), pipe) != nullptr) {
        // DO NOTHING (or log somewhere else)
        std::cout << ppenbuffer << "\n";
    }

    PCLOSE(pipe);
    //> /dev/null 2>&1
}


void Core::devTarget(const std::vector<std::string>& targets, const std::string& pname) {
    std::string root = getProjectRoot(pname);

    if (!targets.empty()) {
        std::string target = targets.at(0);
        if (target == "web") {
            if(targets.size() > 1) {
                std::string flag = targets.at(1);
                cerr << "Unknown Flag: " << flag << endl;
                exit(1);
            }
            std::cout << "\n☀ Helios v0.1.0\n\n";

            long total_time = 0;

            // ---------------- BUILD STAGE ----------------
            try {
                total_time += RunStage("Compiling project...", [&]() {
                    builder();
                });
            } catch (std::exception& e) {
                std::cerr << "\nBuild failed: " << e.what() << "\n";
                exit(1);
            }

            std::cout << "\n";

            // ---------------- SERVER SETUP ----------------
            try {
                std::mutex build_mutex;

                uWS::App app;
                uWS::Loop* loop = nullptr;

                /* ---------- HTTP ---------- */
                app.get("/*", [&](uWS::HttpResponse<false>* res,
                                uWS::HttpRequest* req) {

                    loop = uWS::Loop::get();

                    std::string_view targetview = req->getUrl();
                    std::string target(targetview);
                    if (target.empty() || target == "/")
                        target = "/index.html";

                    std::string full_path;

                    if (target.ends_with(".png") || target.ends_with(".jpg") ||
                        target.ends_with(".jpeg") || target.ends_with(".svg") ||
                        target.ends_with(".ico") || target.ends_with(".gif") ||
                        target.starts_with("/static/")) {
                        full_path = PUBLIC_ROOT + target;
                    } else {
                        full_path = WEB_ROOT + target;
                    }

                    std::string body;

                    if (!read_file(full_path, body)) {
                        full_path = WEB_ROOT + "/index.html";
                        if (!read_file(full_path, body)) {
                            res->writeStatus("404 Not Found")->end("Not found");
                            return;
                        }
                    }

                    res->writeHeader("Content-Type", mime_type(full_path))
                    ->end(body);
                });

                /* ---------- WEBSOCKET ---------- */
                app.ws<int>("/ws", {
                    .open = [](auto* ws) {
                        ws->subscribe("reload");
                    },
                    .message = [](auto* ws, std::string_view msg, uWS::OpCode) {
                        if (msg == "ping")
                            ws->send("pong", uWS::OpCode::TEXT);

                        std::cout << "[Helios] : " << msg << "\n";
                    }
                });

                // ---------------- LISTEN STAGE ----------------
                total_time += RunStage("Starting HTTP server...", [&]() {

                    app.listen(8000, [&](auto* token) {
                        if (!token)
                            throw std::runtime_error("Failed to bind port 8000");

                        loop = uWS::Loop::get();

                        static FileWatcher watcher(
                            "./", {".js", ".css", ".ink"},
                            [&](const std::string& ext) {

                                std::lock_guard<std::mutex> lock(build_mutex);

                                bool buildErr = false;

                                std::string errStr;

                                if (ext == ".ink") {
                                    RunHotReloadStage(buildErr, errStr, [&]() {
                                        builder();
                                    });
                                }

                                loop->defer([&app, buildErr, ext, errStr]{
                                    if (buildErr) {
                                        app.publish("reload", "Error", uWS::OpCode::TEXT);
                                        app.publish("reload", errStr, uWS::OpCode::TEXT);
                                         std::cout << termcolor::red     << "✔ Sent error to clients\n\n"   << termcolor::reset << std::endl;
                                    } else {
                                        app.publish("reload", "r", uWS::OpCode::TEXT);
                                        if(ext != ".ink") {
                                            std::cout << "✔ Sent reload clients\n\n";
                                        }
                                    }
                                });
                            }
                        );

                        watcher.start();
                    });
                });

                // ---------------- SUMMARY ----------------
                std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
                std::cout << "  ➜  Local:   http://localhost:8000\n";
                std::cout << "  ➜  Mode:    Development\n";
                std::cout << "  ➜  Watch:   Enabled\n\n";
                std::cout << "✨ Ready in " << total_time << "ms\n\n";

                // ---------------- RUN (BLOCKING) ----------------
                app.run();

            } catch (std::exception& e) {
                std::cerr << "\nServer failed: " << e.what() << "\n";
                exit(1);
            }



            //string cmd = "cd " + root + "/web && emcc main.cpp -o main.js -sEXPORTED_FUNCTIONS='[\"_main\"]' -sEXPORTED_RUNTIME_METHODS=[ccall,cwrap] -sALLOW_MEMORY_GROWTH";
            
        } else if (target == "android") {
            if(targets.size() > 1) {
                std::string flag = targets.at(1);
                if(flag == "-b") {
                    cout << "[android] Building for development... \n";
                } else {
                    if (flag == "web" && flag == "android" && flag == "ios") {
                        cerr << "cant run multiple targets!!" << endl;
                    } else {
                        cerr << "Unknown Flag: " << flag << endl;
                        exit(1);
                    }
                }
            }
            cout << "[Android] running for development...\n";
            cout << "[android] (to integrate with Gradle/ADB later)\n";
        } else if (target == "ios") {
            if(targets.size() > 1) {
                std::string flag = targets.at(1);
                if(flag == "-b") {
                    cout << "[ios] Building for development... \n";
                } else {
                    if (flag == "web" && flag == "android" && flag == "ios") {
                        cerr << "cant run multiple targets!!" << endl;
                    } else {
                        cerr << "Unknown Flag: " << flag << endl;
                        exit(1);
                    }
                }
            }
            cout << "[ios] running for development...\n";
            cout << "[ios] (to integrate with Xcode later)\n";
        } else {
            cerr << "[ERROR]: Unknown Target: " << target << endl;
            exit(1);
        }
    }
}


// --------------------- Run Production ---------------------
void Core::runTarget(const std::vector<std::string>& targets, const std::string& pname) {
    std::string root = getProjectRoot(pname);

    if (!targets.empty()) {
        std::string target = targets.at(0);
        if (target == "web") {
            if(targets.size() > 1) {
                std::string flag = targets.at(1);
                if(flag == "-b") {
                    builder();
                    cout << "[PRODUCTION][web] Compiling for production... [PRODUCTION]\n";
                } else {
                    if (flag != "web" && flag != "android" && flag != "ios") {
                    cerr << "Unknown Flag: " << flag << endl;
                    exit(1); 
                    }
                }
            }
            cout << "[PRODUCTION][web] running for production... [PRODUCTION]\n";
            std::string cmd = "cd web && python -m http.server 8000";
            //std::string cmd = "cd " + root + "/web && emcc main.cpp -o main.js -sEXPORTED_FUNCTIONS='[\"_main\"]' -sEXPORTED_RUNTIME_METHODS=[ccall,cwrap] -sALLOW_MEMORY_GROWTH";
            system(cmd.c_str());
        } else if (target == "android") {
            if(targets.size() > 1) {
                std::string flag = targets.at(1);
                if(flag == "-b") {
                    cout << "[PRODUCTION][android] Building for production... \n";
                } else {
                    if (flag != "web" && flag != "android" && flag != "ios") {
                        cerr << "Unknown Flag: " << flag << endl;
                        exit(1);
                    }
                }
            }
            cout << "[PRODUCTION][android] running for production...[PRODUCTION]\n";
            cout << "[PRODUCTION][android] (to integrate with Gradle/ADB later)[PRODUCTION]\n";
        } else if (target == "ios") {
            if(targets.size() > 1) {
                std::string flag = targets.at(1);
                if(flag == "-b") {
                    cout << "[PRODUCTION][android] Building for production... \n";
                } else {
                    if (flag == "web" && flag == "android" && flag == "ios") {
                        cerr << "Unknown Flag: " << flag << endl;
                        exit(1);
                    }
                }
            }
            cout << "[PRODUCTION][ios] running for production...[PRODUCTION]\n";
            cout << "[PRODUCTION][ios] (to integrate with Xcode later)[PRODUCTION]\n";
        } else {
            cerr << "[ERROR]: Unknown Target: " << target << endl;
            exit(1);
        }
    }
}

// --------------------- Build ---------------------
void Core::buildTarget(const std::vector<std::string>& targets, const std::string& pname) {
    std::string root = getProjectRoot(pname);

    for (auto& target : targets) {
        if (target == "web") {
            builder();
            cout << "[web] Compiled Web Engine..... \n";
        } else if (target == "android") {
            cout << "[android] Compiling MainActivity...\n";
        } else if (target == "ios") {
            cout << "[ios] Compiling iOS app...\n";
        } else {
            cerr << "[ERROR]: Unknown Target: " << target << endl;
            exit(1);
        }
    }
}



// --------------------- Clean ---------------------
void Core::cleanProject(const std::string& pname) {
    std::string root = getProjectRoot(pname);
    if (fs::exists("web/generated.cpp")) {
        fs::remove("web/generated.cpp");
        cout << "[helios] cleaned project folder: " << root << endl;
    }
}

