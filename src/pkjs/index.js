// PebbleKit JS for Gratitude Journal
// Handles data export to phone/computer

// Export data to JSON format
function exportToJSON(entries, stats) {
  const exportData = {
    version: "0.1.1",
    export_date: new Date().toISOString(),
    entries: entries,
    stats: stats
  };

  const jsonString = JSON.stringify(exportData, null, 2);
  console.log("Export JSON generated:", jsonString.length, "bytes");

  // In a full implementation, this would:
  // 1. Save to phone storage
  // 2. Share via email/messaging
  // 3. Upload to cloud service
  // 4. Display share dialog

  return jsonString;
}

// Export data to Markdown format
function exportToMarkdown(entries, stats) {
  let markdown = "# Gratitude Journal Export\n\n";
  markdown += `**Date**: ${new Date().toLocaleDateString()}\n`;
  markdown += `**Total Entries**: ${stats.total_entries}\n`;
  markdown += `**Current Streak**: ${stats.current_streak} days\n\n`;
  markdown += "---\n\n";

  // Sort entries by date (newest first)
  const sortedEntries = entries.sort((a, b) => b.date - a.date);

  sortedEntries.forEach(entry => {
    const date = new Date(entry.date * 1000);
    markdown += `## ${date.toLocaleDateString()}\n`;
    markdown += `**Mood**: ${entry.mood_label}\n\n`;
    markdown += `> ${entry.text}\n\n`;
    markdown += "---\n\n";
  });

  console.log("Export Markdown generated:", markdown.length, "bytes");
  return markdown;
}

// Listen for messages from the watch
Pebble.addEventListener('appmessage', function(e) {
  console.log("AppMessage received:", JSON.stringify(e.payload));

  // Handle export request
  if (e.payload.export_request) {
    console.log("Export requested");

    // In a full implementation, would:
    // 1. Request all entries from watch via AppMessage
    // 2. Format as JSON/Markdown
    // 3. Save or share the file

    // Send acknowledgment back to watch
    Pebble.sendAppMessage({
      export_status: 1  // 1 = success, 0 = failure
    });
  }
});

// App is ready
Pebble.addEventListener('ready', function(e) {
  console.log("PebbleKit JS ready!");
});

// Export functions for testing
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    exportToJSON,
    exportToMarkdown
  };
}
