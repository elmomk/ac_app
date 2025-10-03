# Use nginx as base image
FROM nginx:alpine

# Copy the static HTML file to nginx
COPY index.html ./images manifest.json sw.js /usr/share/nginx/html/
# Expose port 8001
EXPOSE 80

# Start Nginx
CMD ["nginx", "-g", "daemon off;"]
