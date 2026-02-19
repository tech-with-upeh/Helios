# @stylesheet homepage {
#     mydiv =  {
#         height: "50px;",
#         width: "100%;",
#         position: "fixed;",
#         top: "0px;",
#         display: "flex;",
#         "align-items": "center;",
#         "justify-content": "space-between;",
#         "border-bottom-right-radius": "20px;",
#         "border-bottom-left-radius": "20px;",
#         "z-index": "999"
#     }

#     brandlogo = {
#         height: "30px;",
#         width: "30px;",
#         "object-fit": "contain;"

#     }
    
#     mydivtext = {
#         "font-weight": "bold;",
#         "font-size": "20px;",
#         "text-align": "center;",
#         "user-select": "none;",
#         "cursor": "pointer;"
#     }
#     bgimg = {
#         "position": "absolute",
#         "top": "0",
#         "left": "0px",
#         "width": "100%",
#         "height": "100%",
#         "object-fit": "cover",
#         "z-index": "1",
#         "opacity": "0.2"
#     }
#     media("width" > "992px") {
#         "#hero" {
#             "flex-direction": "row;"
#         }

#         "#hero-terminal" {
#             "width": "40% !important;",
#             "height": "55% !important;"
#         }
#     }
# }

# page("Helios ~ Your Full Stack FrmeWork", style={
#     "background-color": "black",
#     "color": "white",
#     "padding": "0px",
#     "margin": "0px"
# }) {
#     addStyle(homepage)
#     h = Platform().height
#     w = Platform().width
#     @state num : 0
#     @state navh : "0px"
#     @state navpad : "0px"
#     canvas("bg", height=h, width=w)
#     img("box-bg.jpg", cls="bgimg")
 
#     view("mydiv", cls="mydiv", onclick=(num) { 
#         num = num + 1
#         print(num) 
#     }) {
#         view("nav-start", cls="navstart") {
#             img("logo-wh.png", cls="brandlogo")
#             text("Helios", cls="brandname")
#         }
#         view("nav-end", cls="navend") {
#             text(to_str(num), cls="mydivtext")
#             img("nav.png",onclick=(navh, navpad) {
#                 if navh == "0px" {
#                     navh = "150px"
#                     navpad = "20px"
#                 }else {
#                     navh = "0px"
#                     navpad = "0px"
#                 }
#             }, cls="navicon")
#             view("nav-menu",style={
#                 "padding": navpad,
#                 "height": navh
#         }, cls="navmenu") {
#                 text(navh)
#                 text("Docs")
#                 text("GitHub")
#             }
#         }
#     } 
#     view("hero", cls="hero") {
#         view("hero-start", cls="herostart") {
#             text("Build Once, Render Everywhere", cls="herotitle")
#             text("Your Full Stack Framework", style={
#                 "font-size": "20px;",
#                 "color": "#888;"
#             })

#             view("herobtn") {
#                 text("Download Helios")
#             }

#             view("seedocs") {
#                 text("see docs")
#             }
#         }
#         view("hero-terminal", style={
#                 "background-color": "#222;",
#                 "padding": "20px;",
#                 "border-radius": "15px;",
#                 "font-family": "monospace;",
#                 "font-size": "14px;"
#             }) {
#                 view("terminal-header", style={
#                     "display": "flex;",
#                     "align-items": "center;",
#                     "margin-bottom": "10px;"
#                 }) {
#                     view("terminal-header-start", cls="terminalheaderstart", style={
#                         "display": "flex;",
#                         "align-items": "center;",
#                         "border-radius": "50%;",
#                         "margin-right": "5px;"
#                     }) {
#                         view("terminal-dot", style={
#                         "height": "10px;",
#                         "width": "10px;",
#                         "background-color": "#f00;",
#                         "border-radius": "50%;",
#                         "margin-right": "5px;"
#                         })
#                         view("terminal-dot", style={
#                             "height": "10px;",
#                             "width": "10px;",
#                             "background-color": "#0f0;",
#                             "border-radius": "50%;",
#                             "margin-right": "5px;"
#                         })
#                         view("terminal-dot", style={
#                             "height": "10px;",
#                             "width": "10px;",
#                             "background-color": "#ff0;",
#                             "border-radius": "50%;",
#                             "margin-right": "5px;"
#                         })
#                     }
#                     text("Index.ink", style={
#                         "color": "#888;",
#                         "letter-spacing": "5px;",
#                         "font-size": "12px;"
#                     })
#                 }
#                 view("hr", style={
#                     "height": "1px;",
#                     "background-color": "#333;",
#                     "margin": "10px 0;"
#                 })
#             }
#     }

# }


listofpages = [   "index.ink",


"about.ink", "contact.ink"]

page("Helios ~ Your Full Stack Framework") {
    for(p=0: p < listofpages.len: p++) {
        print(listofpages[p])
    }
    text("Welcome to Helios!")
}

        