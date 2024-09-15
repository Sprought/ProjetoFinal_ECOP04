const fs = require("fs").promises;

(async () => {
  try {
    // Read the content of flag.txt
    const data = await fs.readFile("flag.txt", "utf-8");
    
    // Log the file content
    console.log("File content:", data);
  } catch (err) {
    // Log error if file reading fails
    console.error("Error reading file:", err);
  }
})();
