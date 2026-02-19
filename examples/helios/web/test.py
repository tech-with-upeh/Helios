from PIL import Image

# ----- Config -----
image_path = "2c.png"   # Your image file
output_file = "output.txt" # Optional: save text output
# Characters or emojis from dark → light
chars = "▒"             # simple block
# emojis = "🟫🟧🟨⬜ "        # uncomment for emoji version

# ----- Load and Resize -----
img = Image.open(image_path)
# Adjust width for double-width characters
new_width = img.width // 2
new_height = int(img.height * new_width / img.width)
img = img.resize((new_width, new_height))
# Convert to grayscale
img = img.convert("L")

# ----- Map Pixels to Characters -----
pixels = list(img.getdata())
text_pixels = [chars[p * (len(chars) - 1) // 255] for p in pixels]

# ----- Reconstruct Text Lines -----
lines = [
    "".join(text_pixels[i:i+new_width])
    for i in range(0, len(text_pixels), new_width)
]
text_image = "\n".join(lines)

# ----- Output -----
print(text_image)

# Optional: save to file
with open(output_file, "w", encoding="utf-8") as f:
    f.write(text_image)
