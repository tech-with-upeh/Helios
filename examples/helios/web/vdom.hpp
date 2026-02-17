#pragma once
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
void renderPage(VPage& page, bool statechange=false);
std::string genId();
void diff(const VNode& oldN, const VNode& newN);
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void js_removeInlineCSS(const char* key);
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

    // canvas only
    int width = 300;
    int height = 150;
    std::string canvasid = "main-canvas";

    std::string tag;
    std::string dom_id;
    std::string id;
    std::string text;
    std::vector<VNode> children;
    std::unordered_map<std::string, std::string> attrs;
    std::function<void()> onclick;
    std::string callback_id;

    VNode() = default;
    VNode(std::string t, std::string txt="", std::string_view ink_domid = "") : tag(t), text(txt), dom_id(ink_domid) {
    }

    // Helper methods for building VNodes
    VNode& setText(const std::string& newText) {
        text = newText;
        return *this;
    }

    VNode& setAttr(const std::string& key, const std::string& value) {
        if(key == "id") {
            if (type == VNodeType::CANVAS) {
                 canvasid = value;
            }
            id = value;
        }
        attrs[key] = value;
        return *this;
    }

    VNode& addChild(const VNode& child) {
        children.push_back(child);
        return *this;
    }

    VNode& onClick(std::function<void()> handler) {
        onclick = handler;
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
        return *this;
    }

    VPage& rebuild(std::string msg = "") {
        if (!builder) return *this;
        old_children = children;
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
        int height() {
            return EM_ASM_INT({
                return document.documentElement.clientHeight || document.body.clientHeight;
            });
        }
        int width() {
            return EM_ASM_INT({
                return document.documentElement.clientWidth || document.body.clientWidth;
            });
        }


};

std::string escapeErr(const std::string& input) {
    std::string out;
    for (char c : input) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': break;
            default: out += c;
        }
    }
    return out;
}
std::shared_ptr<VPage> MakeErrorPage() {
    auto ErrorPage = std::make_shared<VPage>();
    ErrorPage->builder = [&](VPage& page, std::string msg) {
        page.setTitle("Helios Error");
        page.addStylesheet("https://cdn.lineicons.com/5.1/line/lineicons.css");
        page.setFavicon("logo.png");

        
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

        VNode text_65("p",escapeErr(msg), "__ink_35");

        view_64.addChild(text_65);
        view_57.addChild(view_64);
        view_56.addChild(view_57);
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
    static void navigate(const std::string& path, bool iserr=false) {
        if(iserr == true) {
            renderPage(MakeErrorPage()->rebuild(path));
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
    static int id = 0;
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

// -------------------- Diff Children --------------------
inline void diffChildren(const VNode& oldN, const VNode& newN) {
    size_t oldSize = oldN.children.size();
    size_t newSize = newN.children.size();
    size_t minSize = std::min(oldSize, newSize);

    // 1. Diff existing nodes
    for (size_t i = 0; i < minSize; i++) {
        diff(oldN.children[i], newN.children[i]);
    }

    // 2. Add new nodes
    for (size_t i = minSize; i < newSize; i++) {
        const auto& child = newN.children[i];
        EM_ASM({
            const parent = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
            if (parent) parent.insertAdjacentHTML('beforeend', UTF8ToString($1));
        }, oldN.dom_id.c_str(), renderToHTML(child).c_str());

        // register callbacks for new nodes
        bindOnClick(const_cast<VNode&>(child));
    }

    // 3. Remove old extra nodes
    for (size_t i = minSize; i < oldSize; i++) {
        const auto& child = oldN.children[i];
        EM_ASM({
            const el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
            if (el) el.remove();
        }, child.dom_id.c_str());
    }
}

// -------------------- Diff a VNode --------------------
inline void diff(const VNode& oldN, const VNode& newN) {
    if (oldN.type == VNodeType::CANVAS || newN.type == VNodeType::CANVAS) {
        // For canvas, just replace
        if (oldN.canvasid != newN.canvasid || oldN.width != newN.width || oldN.height != newN.height) {
            EM_ASM({
                const el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
                if (el) el.outerHTML = UTF8ToString($1);
            }, oldN.dom_id.c_str(), renderToHTML(newN).c_str());
        }
        return;
    }

    if (oldN.tag != newN.tag) {
        EM_ASM({
            const el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
            if (el) el.outerHTML = UTF8ToString($1);
        }, oldN.dom_id.c_str(), renderToHTML(newN).c_str());

        return;
    }

    if (oldN.text != newN.text) {
        js_setText(oldN.dom_id.c_str(), newN.text.c_str());
         
    }

    // Attributes
    for (auto& [k, v] : newN.attrs) {
        auto it = oldN.attrs.find(k);
        if (it == oldN.attrs.end() || it->second != v) {
            js_setAttr(oldN.dom_id.c_str(), k.c_str(), v.c_str());
        }
    }

    for (auto& [k, v] : oldN.attrs) {
     // std::cout << " old: " << k << " old: " << v << std::endl;
        if (newN.attrs.find(k) == newN.attrs.end()) {
            js_removeAttr(oldN.dom_id.c_str(), k.c_str());
        }
    }

    // Children
    diffChildren(oldN, newN);
}

// -------------------- Render Page --------------------
inline void renderPage(VPage& page, bool statechange) {
    // Set as current page for callbacks
    GlobalState::setCurrentPage(&page);

    if (statechange) {
        size_t n = std::min(page.old_children.size(), page.children.size());
        for (size_t i = 0; i < n; i++) {
            diff(page.old_children[i], page.children[i]);
        }

        // Add new children
        for (size_t i = n; i < page.children.size(); i++) {
            auto& child = page.children[i];
            js_insertHTML(renderToHTML(child).c_str());
            bindOnClick(child);
        }

        // Remove extra old children
        for (size_t i = n; i < page.old_children.size(); i++) {
            EM_ASM({
                const el = document.querySelector('[data-ink-id="' + UTF8ToString($0) + '"]');
                if (el) el.remove();
            }, page.old_children[i].dom_id.c_str());
        }

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
}