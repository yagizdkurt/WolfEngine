# WolfEngine — In-Code Doxygen Documentation Standard

## 1. Class / Struct Block

Placed immediately above the class declaration.

### Structure

```
/**
 * # <Class/Struct Name> <Category>
 *
 * <Tagline>
 *
 * <Body paragraph 1>
 * <Body paragraph 2 to 5 (optional)>
 *
 * ### Example Usage (optional):
 *
 * ~~~cpp
 * <code or 'None'>
 * ~~~
 *
 * ### Notes:
 * - <note or 'None'>
 */
```

### Generation Rules (Strict)

| Section        | Rule |
|----------------|------|
| Heading        | Format: `# <Name>  <Category>` |
| Heading        | Category ∈ {Component, System, Manager, Module, Utility, Asset} |
| Heading        | Exactly **two spaces** between name and category |
| Tagline        | Exactly **one sentence** |
| Tagline        | ≤ 15 words |
| Tagline        | Must start with a **present-tense verb** |
| Tagline        | Must describe behavior, not identity |
| Tagline        | Example: “Handles sprite rendering using batched draw calls.” |
| Tagline        | Invalid: “This class is responsible for sprite rendering.” |
| Body           | 5 paragraphs max |
| Body           | Each paragraph ≤ 3 lines |
| Body           | Must describe: execution driver, owned/mutated data, engine integration |
| Body           | Must NOT restate obvious members |
| Body           | Must NOT describe trivial getters/setters |
| Example Usage  | If trivial → remove |
| Example Usage  | ≤ 5 lines if present |
| Example Usage  | Must be realistic (no `foo`, `bar`) |
| Notes          | If none → remove |
| Notes          | Use only for constraints, required components, execution order |

### Annotated Example

```cpp
/**
 * # Kinematics  Component
 *
 * Updates entity velocity based on acceleration each fixed step.
 *
 * Runs during the physics phase and mutates `Transform` position
 * using integrated velocity. Does not own memory; operates on ECS data.
 *
 * ### Example Usage:
 *
 * ~~~cpp
 * entity.add<Kinematics>({vx, vy});
 * ~~~
 *
 * ### Notes:
 * - Requires `Transform` component.
 */
```

---

## 2. Public Method Block

Placed immediately above the method declaration.

### Structure

```cpp
/**
 * ## <One-sentence description ending with a period>
 *
 * <Optional extended explanation ONLY if at least one applies:
 *  - side effects are non-trivial
 *  - method depends on engine phase/timing
 *  - method has preconditions or state coupling
 * >
 *
 * @param <name> <description including ownership/lifetime OR "None">
 *
 * @return <description OR "None">
 */
```

### Generation Rules (Strict)

| Section        | Rule |
|----------------|------|
| Opening line   | Must be a single-line comment |
| Opening line   | Must end with a period |
| Opening line   | Must describe behavior (verb-first preferred, not identity) |
| Body           | Optional; include only if at least one applies: side effects, timing dependency, or preconditions |
| Body           | Must be a single paragraph |
| Body           | Maximum 3 lines |
| Body           | Must explain observable behavior, not internal implementation |
| @param         | Always include for every parameter in signature |
| @param         | Format: `@param name description` |
| @param         | Must include ownership/lifetime semantics when relevant |
| @param         | If no parameters exist → remove section |
| @return        | Always include if function returns something |
| @return        | Format: `@return description` |
| @return        | Must describe observable result or state change |
| @return        | If void → remove |
| Ordering       | Must follow strict order: Opening line → Body → @param → @return |
| Style          | No redundant repetition of obvious getters/setters or member fields |

### Examples

```cpp
/** ## Advances the animation by one frame based on delta time accumulation.
 *
 *  @param deltaTime  Time step in seconds used for frame progression.
 */
void Update(float deltaTime);

/** ## Returns the total number of frames in the currently active animation.
 *
 *  @return Frame count of the active animation, or 0 if none is set.
 */
int GetFrameCount() const;

/** ## Immediately jumps to a specific frame in the current animation.
 *
 *  @param frameIndex  Zero-based index of the target frame.
 */
void SetFrame(int frameIndex);

/** ## Returns the current playback frame index.
 *
 *  @return Zero-based index of the active frame.
 */
int GetCurrentFrame() const;

/** ## Restarts the currently active animation from the beginning.
 *
 *  Resets internal time accumulator and sets frame index to 0.
 */
void Restart();
```

---

## 3. Public Data Member Inline Comment

Single-line members use a trailing `///<` comment.  

```cpp
class Animator : public Component {
public:
    bool looping = true;           ///< If false, playback stops on the last frame.
    uint8_t speedMultiplier = 1;   ///< Scales frame advancement; 2 = double speed.
};
```

---

## 4. What to Omit

Do **not** document:

- Private fields or internal helpers.
- Facts already expressed by the type signature (e.g. `@param enabled  The enabled flag.`).
- Implementation details that change frequently — those belong in inline comments, not the public API block.

---

## 5. Category Labels (Quick Reference)

| Label | Use for |
|---|---|
| `Component` | Derives from `Component`; attaches to a `GameObject`. |
| `Module` | Implements `IModule`; registered in `ModuleSystem`. |
| `System` | Core-owned singleton (e.g. `SoundManager`, `Renderer`). |
| `Manager` | Stateful coordinator that doesn't fit Module lifecycle. |
| `Utility` | Stateless helpers, math, converters, etc. |
| `Asset` | Plain data types loaded or generated offline (e.g. `WE_AnimationRaw`). |
