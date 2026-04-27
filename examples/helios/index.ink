include "file"


op.ioo = 1

 page("Helios ~ Your Full Stack Framework") {
     h = Platform().height
     if (h > 992.0) {
         print("This is a desktop device")
     } else {
         print("This is a mobile device")
     }
     text("Welcome to Helios!") 
 }
