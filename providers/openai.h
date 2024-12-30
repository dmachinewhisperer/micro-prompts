/**
 * // API endpoint
const url = 'https://api.openai.com/v1/chat/completions';

// Request body structure
const request = {
  model: "gpt-4-vision-preview", // or "gpt-4", "gpt-3.5-turbo" etc.
  messages: [
    {
      role: "system",
      content: "You are a helpful assistant." // Optional system message
    },
    {
      role: "user",
      content: [
        {
          type: "text",
          text: "Your prompt here"
        },
        // Optional image - only include for vision requests
        {
          type: "image_url",
          image_url: {
            url: "data:image/jpeg;base64,YOUR_BASE64_IMAGE", // or direct image URL
            detail: "auto" // or "low" or "high"
          }
        }
      ]
    }
  ],
  max_tokens: 2048,
  temperature: 0.7,
  top_p: 0.95,
  frequency_penalty: 0,
  presence_penalty: 0
};
 */