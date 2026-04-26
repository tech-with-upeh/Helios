@stylesheet homepage {
    mydiv =  {
        height: "70px;",
        width: "100%;",
        position: "fixed;",
        top: "0px;",
        display: "flex;",
        "align-items": "center;",
        "justify-content": "space-between;",
        "z-index": "999",
        "transition": "background-color 0.3s ease;"
    }

    brandlogo = {
        height: "30px;",
        width: "30px;",
        "object-fit": "contain;"

    }
    
    mydivtext = {
        "font-weight": "bold;",
        "font-size": "20px;",
        "text-align": "center;",
        "user-select": "none;",
        "cursor": "pointer;"
    }
    bgimg = {
        "position": "absolute",
        "top": "0",
        "left": "0px",
        "width": "100%",
        "height": "100%",
        "object-fit": "cover",
        "z-index": "1",
        "background-color": "#191a1f8c",
        "background-image": "linear-gradient(to right, rgba(140, 43, 238, 0.1) 1px, transparent 1px), linear-gradient(to bottom, rgba(140, 43, 238, 0.1) 1px, transparent 1px)",   
        "background-size": "40px 40px"
    }
    media("width" > "992px") {
        "#hero" {
            "flex-direction": "row;"
        }

        "#hero-terminal" {
            "width": "40% !important;",
            "height": "fit-content !important;"
        }

        ".herotitle, .herotitlespan" {
            "font-size": "4em"
        }

        ".features #feature-list" {
            "flex-direction": "row !important;"
        }

        ".features #feature-list #feature-grp" {
            "flex-direction": "row !important;"
        }
    }
}

page("Helios ~ Your Full Stack FrmeWork", style={
    "background-color": "black",
    "color": "white",
    "padding": "0px",
    "margin": "0px"
}) {
    addStyle(homepage)
    h = Platform().height
    w = Platform().width
    @state num : 0
    @state navh : "0px"
    @state navpad : "0px"
    @state scrollYState : 0.0
    @state navBgColor : "transparent"

    

    listener(scroll) {
        scrollYState = Platform().scrollY
        if(scrollYState > 50.0) {
            navBgColor = "rgb(114, 52, 230);"
        } else {
            navBgColor = "transparent"
        }
    }

    listener(resize) {}

    canvas("bg", height=h, width=w)
    view("herobg", cls = "bgimg")
 
    view("mydiv", cls="mydiv", onclick=(num) { 
        num = num + 1
        print(num) 
    }, style={
        "background-color": navBgColor
    }) {
        view("nav-start", cls="navstart") {
            img("logo-wh.png", cls="brandlogo")
            text("Helios", cls="brandname")
        }


        


        
        
        if w > 992.0 {
            view("nav-end", cls="navend") {
                view("navdesk-med") {
                    text("Docs")
                    text("GitHub")
                    input("Search docs...", cls="searchinput")
                }

                view("mavdesk-end") {
                     view("navdesk-btn") {
                        text("Download Helios")
                    }
                }
            }
        } else {
            view("nav-end", cls="navend mobilenav") {
                text(to_str(num), cls="mydivtext")
                img("nav.png",onclick=(navh, navpad) {
                    print("clicked")
                    if navh == "0px" {
                        navh = "150px"
                        navpad = "20px"
                    }else {
                        navh = "0px"
                        navpad = "0px"
                    }
                }, cls="navicon")
                view("nav-menu",style={
                    "padding": navpad,
                    "height": navh
                }, cls="navmenu") {
                    text(navh)
                    text("Docs")
                    text("GitHub")
                }
            }
        }

    } 
    view("hero", cls="hero") {
        view("hero-start", cls="herostart") {
            view("herotxt", cls="herotxt") {
               text("Build Once, Render ", cls="herotitle")
               text("Everywhere", cls="herotitlespan") 
            }
            text("Your Full Stack Framework", style={
                "font-size": "20px;",
                "color": "#888;"
            })

            view("herobtn") {
                text("Download Helios")
            }

            view("seedocs") {
                text("see docs")
            }
        }
        view("hero-terminal", style={
                "background-color": "#222;",
                "padding": "20px;",
                "border-radius": "15px;",
                "font-family": "monospace;",
                "font-size": "14px;"
            }) {
                view("terminal-header", style={
                    "display": "flex;",
                    "align-items": "center;",
                    "margin-bottom": "10px;",
                    height: "6%;",
                    width: "100%"
                }) {
                    view("terminal-header-start", cls="terminalheaderstart", style={
                        "display": "flex;",
                        "align-items": "center;",
                        "border-radius": "50%;",
                        "margin-right": "5px;"
                    }) {
                        view("terminal-dot", style={
                        "height": "0.75rem;",
                        "width": "0.75rem;",
                        "background-color": "#f00;",
                        "border-radius": "50%;",
                        "margin-right": "5px;"
                        })
                        view("terminal-dot", style={
                            "height": "0.75rem;",
                            "width": "0.75rem;",
                            "background-color": "#0f0;",
                            "border-radius": "50%;",
                            "margin-right": "5px;"
                        })
                        view("terminal-dot", style={
                            "height": "0.75rem;",
                            "width": "0.75rem;",
                            "background-color": "#ff0;",
                            "border-radius": "50%;",
                            "margin-right": "5px;"
                        })
                    }
                    text("Index.ink", style={
                        "color": "#888;",
                        "letter-spacing": "5px;",
                        "font-size": "12px;"
                    })
                }
                view("hr", style={
                    "height": "1px;",
                    "background-color": "#333;",
                    "margin": "10px 0;"
                })
                view("terminal-content", cls="terminalcontent") {
                    view("terminal-line", cls="terminalline") {
                        text("1", cls="terminallineno")
                        view("terminal-page-line", style={
                            "display": "flex;",
                            "align-items": "center;"
                        }) {
                            text("page", style={
                                "color": "rgb(114, 52, 230);"
                            }, cls="codeeditor")
                            text("('My Index Page') {", cls="codeeditor")
                        }
                    }
                    view("terminal-line", cls="terminalline") {
                        text("2", cls="terminallineno")
                        view("terminal-view-line", style={
                            "display": "flex;",
                            "align-items": "center;"
                        }) {
                            text("    view", style={
                                "color": "rgb(114, 52, 230);"
                            }, cls="codeeditor")
                            text(    "('mydiv', style={", cls="codeeditor")
                        }
                    }
                    
                    view("terminal-line", cls="terminalline") {
                        text("3", cls="terminallineno")
                        text(        "        'background-color': 'black',", cls="codeeditor")
                    }
                    view("terminal-line", cls="terminalline") {
                        text("4", cls="terminallineno")
                        text(        "        'color': 'white',", cls="codeeditor")
                    }
                    view("terminal-line", cls="terminalline") {
                        text("5", cls="terminallineno")
                        text(        "        'padding': '20px',", cls="codeeditor")
                    }
                    view("terminal-line", cls="terminalline") {
                        text("6", cls="terminallineno")
                        text(        "        'border-radius': '10px;'", cls="codeeditor")
                    }
                    view("terminal-line", cls="terminalline") {
                        text("7", cls="terminallineno")
                        text("}) {", cls="codeeditor")
                    }
                    view("terminal-line", cls="terminalline") {
                        text("8", cls="terminallineno")
                        view("terminal-page-line", style={
                            "display": "flex;",
                            "align-items": "center;"
                        }) {
                            text("        text", style={
                                "color": "rgb(114, 52, 230);"
                            }, cls="codeeditor")
                            text("('Hello World', cls='herotext')", cls="codeeditor")
                        }
                    }
                    view("terminal-line", cls="terminalline") {
                        text("9", cls="terminallineno")
                        text("    }", cls="codeeditor")
                    }
                }
            }
    }

    view("features", cls="features") {
        text("Features ", style={
            "font-size": "18px;",
            "font-weight": "bold;"
        })

        view("feature-list", cls="featurelist") {
            view("feature-grp") {
                view("feature-item", cls="featureitem") {
                    view("feature-icon", cls="lni lni-rocket-5")
                    text("Blazing Fast Performance")
                }
                view("feature-item", cls="featureitem") {
                    view("feature-icon", cls="lni lni-colour-palette-3")
                    text("Rich Component Library")
                }
            }

            view("feature-grp") {
                view("feature-item", cls="featureitem") {
                    view("feature-icon", cls="lni lni-globe-1")
                    text("Cross-Platform Support")
                }

                view("feature-item", cls="featureitem") {
                    view("feature-icon", cls="lni lni-code-1")
                    text(" clean and intuitive syntax")
                }
            }
        }
    }

}


# page("Helios ~ Your Full Stack Framework") {
#     h = Platform().height
#     if (h > 992.0) {
#         print("This is a desktop device")
#     } else {
#         print("This is a mobile device")
#     }
#     text("Welcome to Helios!") 
# }

        
