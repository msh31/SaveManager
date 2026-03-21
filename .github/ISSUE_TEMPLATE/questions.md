name: Question
description: General questions about SaveManager.
labels:
- question

body:
- type: markdown
  attributes:
    value: Before asking a question, [please search to see if it has already been answered](https://github.com/msh31/SaveManager/issues?q=is%3Aissue+label%3Aquestion).

- type: textarea
  attributes:
    label: Question
    description: What would you like to know?
    placeholder: Your question here
  validations:
    required: true

- type: input
  attributes:
    label: SaveManager Version
    placeholder: 1.2.0
    description: Optional, but helps provide context.
  validations:
    required: false
